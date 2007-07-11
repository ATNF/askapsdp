#include <gridding/WProjectVisGridder.h>

#include <conrad/ConradError.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>

using namespace conrad;

namespace conrad
{
  namespace synthesis
  {

    WProjectVisGridder::WProjectVisGridder(const double wmax, const int nwplanes,
      const double cutoff, const int overSample)
    {
      CONRADCHECK(wmax>0.0, "Baseline length must be greater than zero");
      CONRADCHECK(nwplanes>0, "Number of w planes must be greater than zero");
      CONRADCHECK(overSample>0, "Oversampling must be greater than 0");
      CONRADCHECK(cutoff>0.0, "Cutoff must be positive");
      CONRADCHECK(cutoff<1.0, "Cutoff must be less than 1.0");
      itsSupport=0;
      itsNWPlanes=2*nwplanes+1;
      itsWScale=wmax/double(nwplanes);
      itsOverSample=overSample;
      itsCutoff=cutoff;
    }

    WProjectVisGridder::~WProjectVisGridder()
    {
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    void WProjectVisGridder::initConvolutionFunction(IDataSharedIter& idi, 
      const casa::Vector<double>& cellSize,
      const casa::IPosition& shape)
    {
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = idi->uvw().size();
      const int nChan = idi->frequency().size();
      itsCMap.resize(nSamples, nChan);
      itsCMap.set(0);
      
      int cenw=(itsNWPlanes-1)/2;
      for (int i=0;i<nSamples;i++)
      {
        double w=(idi->uvw()(i)(2))/(casa::C::c);
        for (int chan=0;chan<nChan;chan++)
        {
          double freq=idi->frequency()[chan];
          itsCMap(i,chan)=cenw+nint(w*freq/itsWScale);
        }
      }
      if(itsSupport!=0) return;
               
      casa::FFTServer<casa::Float,casa::Complex> ffts;
      itsSupport=0;

      int nx=shape(0)/itsOverSample;
      int ny=shape(1)/itsOverSample;
      int cenx=nx/2;
      int ceny=ny/2;
 
      casa::Vector<double> ccfx(nx);
      casa::Vector<double> ccfy(ny);
      for (int ix=0;ix<nx;ix++)
      {
        double nux=std::abs(double(ix-cenx))/double(nx/2);
        ccfx(ix)=grdsf(nux)/double(nx);
      }
      for (int iy=0;iy<ny;iy++)
      {
        double nuy=std::abs(double(iy-ceny))/double(ny/2);
        ccfy(iy)=grdsf(nuy)/double(nx);
      }
      
      // Now we step through the w planes, starting the furthest
      // out. We calculate the support for that plane and use it
      // for all the others.
      // We pad here to do sinc interpolation of the convolution
      // function in uv space
      casa::Matrix<casa::Complex> thisPlane(nx*itsOverSample, ny*itsOverSample);
      
      double cellx=1.0/(double(nx)*cellSize(0));
      double celly=1.0/(double(ny)*cellSize(1));


      for (int iw=0;iw<=cenw;iw++) {
         
        thisPlane.set(0.0);
        
        double w=double(iw-cenw)*itsWScale;
        for(int ix=cenx-nx/2;ix<cenx+nx/2;ix++)
        {
          double x2=double(ix-cenx)*cellx;
          x2*=x2;
          for(int iy=ceny-ny/2;iy<ceny+ny/2;iy++)
          {
            double y2=double(iy-ceny)*celly;
            y2*=y2;
            double r2=x2+y2;
            double phase=2.0*casa::C::pi*w*(1.0-sqrt(1.0-r2));
            double wt=ccfx(ix-cenx+nx/2)*ccfy(iy-ceny+ny/2);
            thisPlane(ix,iy)=casa::Complex(wt*cos(phase), wt*sin(phase));
          }
        }
        // Now we have to calculate the Fourier transform to get the
        // convolution function in uv space
        for (int iy=0;iy<itsOverSample*ny;iy++)
        {
          casa::Array<casa::Complex> vec(thisPlane.column(iy));
          ffts.fft(vec, true);
        }
        for (int ix=0;ix<itsOverSample*nx;ix++)
        {
          casa::Array<casa::Complex> vec(thisPlane.row(ix));
          ffts.fft(vec, true);
        }
        // If the support is not yet set, find it and size the
        // convolution function appropriately
        if(itsSupport==0) {
          // Find the support by starting from the edge and
          // working in
          for(int ix=0;ix<itsOverSample*cenx;ix++)
          {
            if(abs(thisPlane(ix,itsOverSample*ceny))>itsCutoff){
              itsSupport=abs(ix-itsOverSample*cenx)/itsOverSample;
              break;
            }
          }
          itsSupport=(itsSupport<nx/2)?itsSupport:nx/2;
          itsCSize=2*(itsSupport+1)*itsOverSample;
          std::cout << "W support = " << itsSupport 
            << " pixels, convolution function size = " << itsCSize << " pixels" 
            << std::endl;
          itsCCenter=itsCSize/2-1;
          itsC.resize(itsCSize, itsCSize, itsNWPlanes);
          itsC.set(0.0);
        }
        // Now cut out the inner part of the convolution function
        for (int iy=-itsSupport*itsOverSample;iy<+itsOverSample*itsSupport;iy++)
        {
          for (int ix=-itsOverSample*itsSupport;ix<+itsOverSample*itsSupport;ix++)
          {
              itsC(ix+itsCCenter,iy+itsCCenter,iw)=
                thisPlane(ix+itsOverSample*cenx,iy+itsOverSample*ceny);
          }
        }
        itsC.xyPlane(itsNWPlanes-1-iw)=casa::conj(itsC.xyPlane(iw));
      }
//      std::cout << "Shape of convolution function = " << itsC.shape() << std::endl;
//      for (int iw=0;iw<itsNWPlanes;iw++) {
//        std::cout << iw << " " << real(casa::max(casa::abs(itsC.xyPlane(iw)))) 
//          << std::endl;
//      }
    }
    
    int WProjectVisGridder::cOffset(int row, int chan)
    {
      return itsCMap(row, chan);
    }

  }
}
