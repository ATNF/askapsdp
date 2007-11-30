#include <gridding/AProjectWStackVisGridder.h>

#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>

#include <fft/FFTWrapper.h>

using namespace conrad;

namespace conrad
{
  namespace synthesis
  {

    AProjectWStackVisGridder::AProjectWStackVisGridder(const double diameter,
        const double blockage, const double wmax, const int nwplanes,
        const int overSample, const int maxSupport, const int maxFeeds,
        const bool frequencyDependent, const std::string& name) :
      WStackVisGridder(wmax, nwplanes), itsReferenceFrequency(0.0),
          itsDiameter(diameter), itsBlockage(blockage),
          itsMaxFeeds(maxFeeds), itsFreqDep(frequencyDependent)

    {
      CONRADCHECK(diameter>0.0, "Blockage must be positive");
      CONRADCHECK(diameter>blockage,
          "Antenna diameter must be greater than blockage");
      CONRADCHECK(blockage>=0.0, "Blockage must be non-negative");
      CONRADCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
      CONRADCHECK(overSample>0, "Oversampling must be greater than 0");
      CONRADCHECK(maxSupport>0, "Maximum support must be greater than 0")
      itsSupport=0;
      itsOverSample=overSample;
      itsMaxSupport=maxSupport;
      itsName=name;
    }

    AProjectWStackVisGridder::~AProjectWStackVisGridder()
    {
    }

    /// Clone a copy of this Gridder
    IVisGridder::ShPtr AProjectWStackVisGridder::clone()
    {
      return IVisGridder::ShPtr(new AProjectWStackVisGridder(*this));
    }

    /// Initialize the indices into the cube.
    void AProjectWStackVisGridder::initIndices(IDataSharedIter& idi)
    {
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = idi->uvw().size();
      int nChan=1;
      if (itsFreqDep)
      {
        nChan = idi->frequency().size();
      }
      const int nPol = idi->rwVisibility().shape()(2);
      itsCMap.resize(nSamples, nPol, nChan);
      itsCMap.set(0);
      /// @todo Select max feeds more carefully

      itsGMap.resize(nSamples, nPol, nChan);
      itsGMap.set(0);
      int cenw=(itsNWPlanes-1)/2;

      for (int i=0; i<nSamples; i++)
      {
        int feed=idi->feed1()(i);
        CONRADCHECK(feed<itsMaxFeeds,
            "Exceeded specified maximum number of feeds");
        CONRADCHECK(feed>-1, "Illegal negative feed number");

        double w=(idi->uvw()(i)(2))/(casa::C::c);

        for (int chan=0; chan<nChan; chan++)
        {
          for (int pol=0; pol<nPol; pol++)
          {
            /// Calculate the index into the convolution functions
            /// Order is (chan, feed)
            itsCMap(i, pol, chan)=chan+nChan*feed;

            /// Calculate the index into the grids
            double freq=idi->frequency()[chan];
            if (itsNWPlanes>1)
            {
              itsGMap(i, pol, chan)=cenw+nint(w*freq/itsWScale);
            }
            else
            {
              itsGMap(i, pol, chan)=0;
            }
            CONRADCHECK(itsGMap(i, pol, chan)<itsNWPlanes,
                "W scaling error: recommend allowing larger range of w");
            CONRADCHECK(itsGMap(i, pol, chan)>-1,
                "W scaling error: recommend allowing larger range of w");
          }
        }
      }
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    /// @todo Make initConvolutionFunction more robust
    void AProjectWStackVisGridder::initConvolutionFunction(IDataSharedIter& idi)
    {
      if (itsSupport!=0)
        return;

      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      int nChan=1;
      if (itsFreqDep)
      {
        nChan = idi->frequency().size();
      }

      /// Get the pointing direction
      casa::Matrix<double> slope;
      findCollimation(idi, slope);

      /// Limit the size of the convolution function since
      /// we don't need it finely sampled in image space. This
      /// will reduce the time taken to calculate it.
      int nx=std::min(itsMaxSupport, itsShape(0));
      int ny=std::min(itsMaxSupport, itsShape(1));

      double cellu=itsUVCellSize(0)/itsOverSample;
      double cellv=itsUVCellSize(1)/itsOverSample;

      int zIndex=0;
      for (int feed=0; feed<itsMaxFeeds; feed++)
      {

        for (int chan=0; chan<nChan; chan++)
        {
          double ax=2.0f*casa::C::pi*cellu*slope(0, feed);
          double ay=2.0f*casa::C::pi*cellv*slope(1, feed);

          /// Make the disk for this channel
          casa::Matrix<casa::Complex> disk(nx, ny);
          disk.set(0.0);

          /// Calculate the size of one cell in m.
          double cell=std::abs(cellu*(casa::C::c/idi->frequency()[chan]));
          double rmax=std::pow(itsDiameter/(2.0*cell), 2);
          double rmin=std::pow(itsBlockage/(2.0*cell), 2);

          /// Calculate the antenna voltage pattern, including the
          /// phase shift due to pointing
          double sumdisk=0.0;
          for (int ix=0; ix<nx; ix++)
          {
            double nux=double(ix-nx/2);
            double nux2=nux*nux;
            for (int iy=0; iy<ny; iy++)
            {
              double nuy=double(iy-ny/2);
              double nuy2=nuy*nuy;
              double r=nux2+nuy2;
              if ((r>=rmin)&&(r<=rmax))
              {
                double phase=ax*nux+ay*nuy;
                disk(ix, iy)=casa::Complex(cos(phase), -sin(phase));
                sumdisk+=1.0;
              }
            }
          }
          CONRADCHECK(sumdisk>0.0, "Integral of disk should be non-zero");
          disk*=casa::Complex(float(nx)*float(ny)/sumdisk);

          /// Now convolve the disk with itself
          fft2d(disk, false);
          //					std::cout << "Feed " << feed
          //						  << ": Peak of primary beam voltage pattern = " << casa::max(casa::abs(disk)) << std::endl;
          for (int ix=0; ix<nx; ix++)
          {
            for (int iy=0; iy<ny; iy++)
            {
              disk(ix, iy)=disk(ix,iy)*conj(disk(ix,iy));
            }
          }
          //					std::cout << "Feed " << feed
          //						  <<": Peak of primary beam power pattern = " << casa::real(casa::max(casa::abs(disk)))
          //						  << std::endl;
          fft2d(disk, true);
          //					std::cout << "Feed " << feed
          //						  << ": Integral of primary beam power pattern = " << disk(nx/2,ny/2) 
          //						  << " (pixels) " << std::endl;
          sumdisk=0.0;
          for (int ix=0; ix<nx; ix++)
          {
            for (int iy=0; iy<ny; iy++)
            {
              sumdisk+=casa::abs(disk(ix, iy));
            }
          }
          disk*=casa::Complex(float(itsOverSample)*float(itsOverSample)/casa::Complex(sumdisk));

          if (itsSupport==0)
          {
            itsSupport=1+2*nint(casa::sqrt(rmax))/itsOverSample;
            CONRADCHECK(itsSupport>0,
                "Unable to determine support of convolution function");
            CONRADCHECK(itsSupport*itsOverSample<nx/2,
                "Overflowing convolution function - increase maxSupport or decrease overSample")
            itsCSize=2*(itsSupport+1)*itsOverSample+1;
            std::cout << "Convolution function support = "<< itsSupport
                << " pixels, convolution function size = "<< itsCSize
                << " pixels"<< std::endl;
            std::cout << "Maximum extent = "<< itsSupport*itsOverSample*cell
                << " (m) sampled at "<< cell << " (m)"<< std::endl;
            itsCCenter=(itsSupport+1)*itsOverSample;
            itsConvFunc.resize(itsMaxFeeds*nChan);
            itsSumWeights.resize(itsMaxFeeds*nChan, itsShape(2), itsShape(3));
            itsSumWeights.set(casa::Complex(0.0));
          }
          zIndex=chan+nChan*feed;

          itsConvFunc[zIndex].resize(itsCSize, itsCSize);
          itsConvFunc[zIndex].set(0.0);
          // Now cut out the inner part of the convolution function and
          // insert it into the convolution function
          for (int iy=0; iy<itsCSize; iy++)
          {
            for (int ix=0; ix<itsCSize; ix++)
            {
              itsConvFunc[zIndex](ix, iy)=
              disk(ix-itsCCenter+nx/2, iy-itsCCenter+ny/2);
            }
          }
        } // chan loop
                } // feed loop
                std::cout << "Shape of convolution function = "<< itsConvFunc[0].shape()
                << " by "<< itsConvFunc.size()<< " planes"<< std::endl;
                if (itsName!="") save(itsName);
              }

              /// To finalize the transform of the weights, we use the following steps:
              /// 1. For each plane of the convolution function, transform to image plane
              /// and multiply by conjugate to get abs value squared.
              /// 2. Sum all planes weighted by the weight for that convolution function.
              void AProjectWStackVisGridder::finaliseWeights(casa::Array<double>& out)
              {

                std::cout << "Calculating sum of weights image"<< std::endl;

                int nx=itsShape(0);
                int ny=itsShape(1);
                int nPol=itsShape(2);
                int nChan=itsShape(3);

                int nZ=itsSumWeights.shape()(0);

                /// We must pad the convolution function to full size, reverse transform
                /// square, and sum multiplied by the corresponding weight
                int cnx=std::min(itsMaxSupport, nx);
                int cny=std::min(itsMaxSupport, ny);
                int ccenx=cnx/2;
                int cceny=cny/2;

                /// This is the output array before sinc padding
                casa::Array<double> cOut(casa::IPosition(4, cnx, cny, nPol, nChan));
                cOut.set(0.0);

                /// Work space
                casa::Matrix<casa::Complex> thisPlane(cnx, cny);

                for (int iz=0; iz<nZ; iz++)
                {
                  thisPlane.set(0.0);

                  // Now fill the inner part of the uv plane with the convolution function
                  // and transform to obtain the image. The uv sampling is fixed here
                  // so the total field of view is itsOverSample times larger than the
                  // original field of view.
                  for (int iy=-itsSupport; iy<+itsSupport; iy++)
                  {
                    for (int ix=-itsSupport; ix<+itsSupport; ix++)
                    {
                      thisPlane(ix+ccenx, iy+cceny)=itsConvFunc[iz](ix*itsOverSample
                          +itsCCenter, iy*itsOverSample+itsCCenter);
                    }
                  }

                  thisPlane*=casa::Complex(double(cnx)*double(cny));
                  /// The peak here should be unity
                  fft2d(thisPlane, false);

                  // Now we need to cut out only the part inside the field of view
                  for (int chan=0; chan<nChan; chan++)
                  {
                    for (int pol=0; pol<nPol; pol++)
                    {
                      casa::IPosition ip(4, 0, 0, pol, chan);
                      casa::Complex wt=itsSumWeights(iz, pol, chan);
                      for (int ix=0; ix<cnx; ix++)
                      {
                        ip(0)=ix;
                        for (int iy=0; iy<cny; iy++)
                        {
                          ip(1)=iy;
                          cOut(ip)+=casa::real(wt*thisPlane(ix, iy)
                              *conj(thisPlane(ix, iy)));
                        }
                      }
                    }
                  }
                }
                fftPad(cOut, out);
              }

              void AProjectWStackVisGridder::fftPad(const casa::Array<double>& in,
                  casa::Array<double>& out)
              {

                int inx=in.shape()(0);
                int iny=in.shape()(1);

                int onx=out.shape()(0);
                int ony=out.shape()(1);

                // Shortcut no-op
                if ((inx==onx)&&(iny==ony))
                {
                  out=in.copy();
                  return;
                }

                CONRADCHECK(onx>=inx, "Attempting to pad to smaller array");
                CONRADCHECK(ony>=iny, "Attempting to pad to smaller array");

                /// Make an iterator that returns plane by plane
                casa::ReadOnlyArrayIterator<double> inIt(in, 2);
                casa::ArrayIterator<double> outIt(out, 2);
                while (!inIt.pastEnd()&&!outIt.pastEnd())
                {
                  casa::Matrix<casa::DComplex> inPlane(inx, iny);
                  casa::Matrix<casa::DComplex> outPlane(onx, ony);
                  casa::convertArray(inPlane, inIt.array());
                  outPlane.set(0.0);
                  fft2d(inPlane, false);
                  for (int iy=0; iy<iny; iy++)
                  {
                    for (int ix=0; ix<inx; ix++)
                    {
                      outPlane(ix+(onx-inx)/2, iy+(ony-iny)/2) = inPlane(ix, iy);
                    }
                  }
                  fft2d(outPlane, true);
                  const casa::Array<casa::DComplex> constOutPlane(outPlane);
                  casa::Array<double> outArray(outIt.array());

                  casa::real(outArray, constOutPlane);

                  inIt.next();
                  outIt.next();
                }
              }

              int AProjectWStackVisGridder::cIndex(int row, int pol, int chan)
              {
                return itsCMap(row, pol, chan);
              }

              void AProjectWStackVisGridder::findCollimation(IDataSharedIter& idi,
                  casa::Matrix<double>& slope)
              {
                casa::Quantum<double>refLon((itsAxes.start("RA")+itsAxes.end("RA"))/2.0,
                    "rad");
                casa::Quantum<double> refLat((itsAxes.start("DEC")+itsAxes.end("DEC"))
                    /2.0, "rad");
                casa::MVDirection out(refLon, refLat);
                const int nSamples = idi->uvw().size();
                slope.resize(2, itsMaxFeeds);
                slope.set(0.0);
                casa::Vector<bool> done(itsMaxFeeds);
                done.set(false);

                // exact formulae for l and m 

                /// @todo Deal with changing pointing
                casa::Vector<double> uvw(3);
                int nDone=0;
                for (int row=0; row<nSamples; row++)
                {
                  int feed=idi->feed1()(row);
                  CONRADCHECK(feed<itsMaxFeeds, "Too many feeds: increase maxfeeds");
                  if (!done(feed))
                  {
                    casa::MVAngle mvLong=idi->pointingDir1()(row).getAngle().getValue()(0);
                    casa::MVAngle mvLat=idi->pointingDir1()(row).getAngle().getValue()(1);
                    //					          std::cout << "Feed " << feed << " points at Right Ascension ";
                    //					          std::cout << mvLong.string(casa::MVAngle::TIME, 8)
                    //					          << ", Declination ";
                    //					          std::cout << mvLat.string(casa::MVAngle::DIG2, 8);
                    //					          std::cout << " (J2000)";
                    casa::MVDirection offset(idi->pointingDir1()(row).getAngle());
                    slope(0,feed)=sin(offset.getLong()-out.getLong())*cos(offset.getLat());
                    slope(1,feed)=sin(offset.getLat())*cos(out.getLat())-
                    cos(offset.getLat())*sin(out.getLat())*cos(offset.getLong()-out.getLong());
                    //					          std::cout << ", offset by " << 180.0*slope(0,feed)/casa::C::pi << " "
                    //					          << 180.0*slope(1,feed)/casa::C::pi << " degrees" << std::endl;

                    done(feed)=true;
                    nDone++;
                    if (nDone==itsMaxFeeds)
                    break;
                  }
                }
                CONRADCHECK(nDone==itsMaxFeeds, "Failed to find pointing for all feeds");
              }

              void AProjectWStackVisGridder::correctConvolution(casa::Array<double>& grid)
              {
              }

            }
          }
