/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///

#include <gridding/WProjectVisGridder.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".gridding");

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <casa/BasicSL/Constants.h>
#include <fft/FFTWrapper.h>

using namespace askap;

#include <cmath>

namespace askap
{
  namespace synthesis
  {

    WProjectVisGridder::WProjectVisGridder(const double wmax,
        const int nwplanes, const double cutoff, const int overSample,
	const int maxSupport, const int limitSupport, const std::string& name)
    {
      ASKAPCHECK(wmax>0.0, "Baseline length must be greater than zero");
      ASKAPCHECK(nwplanes>0, "Number of w planes must be greater than zero");
      ASKAPCHECK(nwplanes%2==1, "Number of w planes must be odd");
      ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
      ASKAPCHECK(cutoff>0.0, "Cutoff must be positive");
      ASKAPCHECK(cutoff<1.0, "Cutoff must be less than 1.0");
      ASKAPCHECK(maxSupport>0, "Maximum support must be greater than 0")
      itsSupport=0;
      itsNWPlanes=nwplanes;
      if (nwplanes>1) {
         itsWScale=wmax/double((nwplanes-1)/2);
      } else {
         itsWScale=1.0;
      }
      itsOverSample=overSample;
      itsCutoff=cutoff;
      itsMaxSupport=maxSupport;
      itsLimitSupport=limitSupport;
      itsName=name;

      itsConvFunc.resize(itsNWPlanes*itsOverSample*itsOverSample);     
    }

    WProjectVisGridder::~WProjectVisGridder()
    {
    }
    
    /// @brief copy constructor
    /// @details It is required to decouple internal arrays in the input
    /// object and the copy.
    /// @param[in] other input object
    WProjectVisGridder::WProjectVisGridder(const WProjectVisGridder &other) :
         SphFuncVisGridder(other), itsWScale(other.itsWScale), 
         itsNWPlanes(other.itsNWPlanes), itsCutoff(other.itsCutoff),
         itsCMap(other.itsCMap.copy()), itsMaxSupport(other.itsMaxSupport),
         itsLimitSupport(other.itsLimitSupport) {}
           

    /// Clone a copy of this Gridder
    IVisGridder::ShPtr WProjectVisGridder::clone()
    {
      return IVisGridder::ShPtr(new WProjectVisGridder(*this));
    }

    /// @brief initialise sum of weights
    /// @details We keep track the number of times each convolution function is used per
    /// channel and polarisation (sum of weights). This method is made virtual to be able
    /// to do gridder specific initialisation without overriding initialiseGrid.
    /// This method accepts no parameters as itsShape, itsNWPlanes, etc should have already
    /// been initialised by the time this method is called.
    void WProjectVisGridder::initialiseSumOfWeights()
    {
       itsSumWeights.resize(itsNWPlanes, itsShape.nelements()>=3 ? itsShape(2) : 1, 
                              itsShape.nelements()>=4 ? itsShape(3) : 1);
       itsSumWeights.set(0.0);
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    void WProjectVisGridder::initIndices(const IConstDataAccessor& acc)
    {
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = acc.nRow();
      const int nChan = acc.nChannel();
      const int nPol = acc.nPol();

      itsCMap.resize(nSamples, nPol, nChan);
      int cenw=(itsNWPlanes-1)/2;
      const casa::Vector<casa::RigidVector<double, 3> > &rotatedUVW = acc.rotatedUVW(getTangentPoint());
      
      for (int i=0; i<nSamples; ++i)
      {
        double w=(rotatedUVW(i)(2))/(casa::C::c);
        for (int chan=0; chan<nChan; ++chan)
        {
          for (int pol=0; pol<nPol; ++pol)
          {
            double freq=acc.frequency()[chan];
            /// Calculate the index into the convolution functions
            if (itsNWPlanes>1)
            {
              itsCMap(i, pol, chan)=cenw+nint(w*freq/itsWScale);
            }
            else
            {
              itsCMap(i, pol, chan)=0;
            }
            if (itsCMap(i, pol, chan)<0)
            {
              ASKAPLOG_WARN_STR(logger, w << " "<< freq << " "<< itsWScale
                  << " "<< itsCMap(i, pol, chan) );
            }
            ASKAPCHECK(itsCMap(i, pol, chan)<itsNWPlanes,
                "W scaling error: recommend allowing larger range of w, you have w="<<w*freq<<" wavelengths");
            ASKAPCHECK(itsCMap(i, pol, chan)>-1,
                "W scaling error: recommend allowing larger range of w, you have w="<<w*freq<<" wavelengths");
          }
        }
      }
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    void WProjectVisGridder::initConvolutionFunction(const IConstDataAccessor&)
    {
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int cenw=(itsNWPlanes-1)/2;

      if (itsSupport>0)
      {
        return;
      }

      itsSupport=0;

      /// These are the actual cell sizes used
      float cellx=1.0/(float(itsShape(0))*itsUVCellSize(0));
      float celly=1.0/(float(itsShape(1))*itsUVCellSize(1));

      /// Limit the size of the convolution function since
      /// we don't need it finely sampled in image space. This
      /// will reduce the time taken to calculate it.
      //      int nx=std::min(itsMaxSupport, itsShape(0));
      //      int ny=std::min(itsMaxSupport, itsShape(1));
      int nx=itsMaxSupport;
      int ny=itsMaxSupport;
      /// We want nx * ccellx = overSample * itsShape(0) * cellx

      int qnx=nx/itsOverSample;
      int qny=ny/itsOverSample;

      // Find the actual cellsizes in x and y (radians) after over
      // oversampling (in uv space)
      float ccellx=float(itsShape(0))*cellx/float(qnx);
      float ccelly=float(itsShape(1))*celly/float(qny);

      casa::Vector<float> ccfx(qnx);
      casa::Vector<float> ccfy(qny);
      for (int ix=0; ix<qnx; ix++)
      {
        float nux=std::abs(float(ix-qnx/2))/float(qnx/2);
        ccfx(ix)=grdsf(nux)/float(qnx);
      }
      for (int iy=0; iy<qny; iy++)
      {
        float nuy=std::abs(float(iy-qny/2))/float(qny/2);
        ccfy(iy)=grdsf(nuy)/float(qny);
      }

      // Now we step through the w planes, starting the furthest
      // out. We calculate the support for that plane and use it
      // for all the others.

      // We pad here to do sinc interpolation of the convolution
      // function in uv space
      casa::Matrix<casa::Complex> thisPlane(nx, ny);
      ASKAPDEBUGASSERT(nx>0);
      ASKAPDEBUGASSERT(ny>0);     

      for (int iw=0; iw<itsNWPlanes; iw++)
      {
        thisPlane.set(0.0);

        const float w=2.0f*casa::C::pi*float(iw-cenw)*itsWScale;
        // Loop over the central nx, ny region, setting it to the product
        // of the phase screen and the spheroidal function
        for (int iy=0; iy<qny; iy++)
        {
          float y2=float(iy-qny/2)*ccelly;
          y2*=y2;
          for (int ix=0; ix<qnx; ix++)
          {
            float x2=float(ix-qnx/2)*ccellx;
            x2*=x2;
            float r2=x2+y2;
	    if(r2<1.0) {
	      float phase=w*(1.0-sqrt(1.0-r2));
	      float wt=ccfx(ix)*ccfy(iy);
	      ASKAPDEBUGASSERT(ix-qnx/2+nx/2 < nx);
	      ASKAPDEBUGASSERT(iy-qny/2+ny/2 < ny);
	      ASKAPDEBUGASSERT(ix+nx/2 >= qnx/2);
	      ASKAPDEBUGASSERT(iy+ny/2 >= qny/2);
	      thisPlane(ix-qnx/2+nx/2, iy-qny/2+ny/2)=casa::Complex(wt*cos(phase), -wt*sin(phase));
	    }
          }
        }
        // At this point, we have the phase screen multiplied by the spheroidal
        // function, sampled on larger cellsize (itsOverSample larger) in image
        // space. Only the inner qnx, qny pixels have a non-zero value

        // Now we have to calculate the Fourier transform to get the
        // convolution function in uv space
        scimath::fft2d(thisPlane, true);

        // Now thisPlane is filled with convolution function
        // sampled on a finer grid in u,v
        //
        // If the support is not yet set, find it and size the
        // convolution function appropriately
        if (itsSupport==0)
        {
          // Find the support by starting from the edge and
          // working in
          
          // cutoff in absolute units
          const double cutoff = casa::abs(thisPlane(nx/2,ny/2))*itsCutoff; 
          ASKAPLOG_INFO_STR(logger, "Convolution function relative cutoff of "<<
              itsCutoff<<" is equivalent to absolute cutoff of "<<cutoff); 
          
          for (int ix=0; ix<nx/2; ix++)
          {
            /// Check on horizontal axis
            if ((casa::abs(thisPlane(ix, ny/2))>cutoff))
            {
              itsSupport=abs(ix-nx/2)/itsOverSample;
              break;
            }
            ///  Check on diagonal
            if ((casa::abs(thisPlane(ix, ix))>cutoff))
            {
              itsSupport=abs(int(1.414*float(ix))-nx/2)/itsOverSample;
              break;
            }
            if (nx==ny)
            {
              /// Check on vertical axis
              if ((casa::abs(thisPlane(nx/2, ix))>cutoff))
              {
                itsSupport=abs(ix-ny/2)/itsOverSample;
                break;
              }
            }
          }
          ASKAPCHECK(itsSupport>0,
              "Unable to determine support of convolution function");
          ASKAPCHECK(itsSupport*itsOverSample<nx/2,
              "Overflowing convolution function - increase maxSupport or decrease overSample")
	  if (itsLimitSupport > 0  &&  itsSupport > itsLimitSupport) {
	    ASKAPLOG_INFO_STR(logger, "Convolution function support = "
	      << itsSupport << " pixels exceeds upper support limit; "
	      << "set to limit = " << itsLimitSupport << " pixels");
	    itsSupport = itsLimitSupport;
	  }
          itsCSize=2*itsSupport+1;
          ASKAPLOG_INFO_STR(logger, "Convolution function support = "
              << itsSupport << " pixels, convolution function size = "
              << itsCSize<< " pixels");
          itsCCenter=(itsCSize-1)/2;
        }
	ASKAPCHECK(itsConvFunc.size()>0, "Convolution function not sized correctly");
        for (int fracu=0; fracu<itsOverSample; ++fracu) {
          for (int fracv=0; fracv<itsOverSample; ++fracv) {
            const int plane=fracu+itsOverSample*(fracv+itsOverSample*iw);
            ASKAPDEBUGASSERT(plane < int(itsConvFunc.size()));
            itsConvFunc[plane].resize(itsCSize, itsCSize);
            itsConvFunc[plane].set(0.0);
            // Now cut out the inner part of the convolution function and
            // insert it into the convolution function
            for (int iy=-itsSupport; iy<itsSupport; ++iy) {
                 for (int ix=-itsSupport; ix<itsSupport; ++ix) {
                      ASKAPDEBUGASSERT((ix + itsCCenter >= 0) && (iy + itsCCenter >= 0));
                      ASKAPDEBUGASSERT(ix+itsCCenter < int(itsConvFunc[plane].nrow()));
                      ASKAPDEBUGASSERT(iy+itsCCenter < int(itsConvFunc[plane].ncolumn()));
                      ASKAPDEBUGASSERT(ix*itsOverSample+fracu+nx/2 >= 0);
                      ASKAPDEBUGASSERT(iy*itsOverSample+fracv+ny/2 >= 0);
                      ASKAPDEBUGASSERT(ix*itsOverSample+fracu+nx/2 < int(thisPlane.nrow()));
                      ASKAPDEBUGASSERT(iy*itsOverSample+fracv+ny/2 < int(thisPlane.ncolumn()));                      
                      itsConvFunc[plane](ix+itsCCenter, iy+itsCCenter) =
                          thisPlane(ix*itsOverSample+fracu+nx/2, iy*itsOverSample+fracv+ny/2);
                 } // for ix
            } // for iy
          } // for fracv
        } // for fracu
        // force normalization for all fractional offsets (or planes)
		for (size_t plane = 0; plane<itsConvFunc.size(); ++plane) {
	         if (itsConvFunc[plane].nelements() == 0) {
				 // this plane of the cache is unused
				 continue;
             }
		 const double norm = sum(casa::real(itsConvFunc[plane]));
		 //		 ASKAPLOG_INFO_STR(logger, "Sum of convolution function = " << norm);
             
	         ASKAPDEBUGASSERT(norm>0.);
	         if(norm>0.) {
	            itsConvFunc[plane]/=casa::Complex(norm);
	         }
        } // for plane					        
      } // for iw
      ASKAPLOG_INFO_STR(logger, "Shape of convolution function = "
          << itsConvFunc[0].shape() << " by "<< itsConvFunc.size() << " planes");
      if (itsName!="")
        save(itsName);
      ASKAPCHECK(itsSupport>0, "Support not calculated correctly");

    }

    int WProjectVisGridder::cIndex(int row, int pol, int chan)
    {
      return itsCMap(row, pol, chan);
    }

    void WProjectVisGridder::fftPad(const casa::Array<double>& in,
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

      ASKAPCHECK(onx>=inx, "Attempting to pad to smaller array");
      ASKAPCHECK(ony>=iny, "Attempting to pad to smaller array");

      /// Make an iterator that returns plane by plane
      casa::ReadOnlyArrayIterator<double> inIt(in, 2);
      casa::ArrayIterator<double> outIt(out, 2);
      while (!inIt.pastEnd()&&!outIt.pastEnd())
      {
        casa::Matrix<casa::DComplex> inPlane(inx, iny);
        casa::Matrix<casa::DComplex> outPlane(onx, ony);
        casa::convertArray(inPlane, inIt.array());
        outPlane.set(0.0);
        scimath::fft2d(inPlane, false);
        for (int iy=0; iy<iny; iy++)
        {
          for (int ix=0; ix<inx; ix++)
          {
            ASKAPDEBUGASSERT(ix+(onx-inx)/2 < int(outPlane.nrow()));
            ASKAPDEBUGASSERT(iy+(ony-iny)/2 < int(outPlane.ncolumn()));
            ASKAPDEBUGASSERT(ix+(onx-inx)/2 >= 0);
            ASKAPDEBUGASSERT(iy+(ony-iny)/2 >= 0);            
            outPlane(ix+(onx-inx)/2, iy+(ony-iny)/2) = inPlane(ix, iy);
          }
        }
        scimath::fft2d(outPlane, true);
        const casa::Array<casa::DComplex> constOutPlane(outPlane);
        casa::Array<double> outArray(outIt.array());

        casa::real(outArray, constOutPlane);

        inIt.next();
        outIt.next();
      }
    }

  }
}
