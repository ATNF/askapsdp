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

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".gridding");

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <fft/FFTWrapper.h>
#include <gridding/UVPattern.h>
#include <gridding/IBasicIllumination.h>

#include <measurementequation/PaddingUtils.h>

using namespace askap;

namespace askap {
  namespace synthesis {
    
    AProjectWStackVisGridder::AProjectWStackVisGridder(const boost::shared_ptr<IBasicIllumination const> &illum,
						       const double wmax, const int nwplanes,
						       const int overSample, const int maxSupport, const int limitSupport, 
						       const int maxFeeds,
						       const int maxFields, const double pointingTol,
						       const bool frequencyDependent, const std::string& name) :
      WStackVisGridder(wmax, nwplanes), itsReferenceFrequency(0.0),
      itsIllumination(illum),
      itsMaxFeeds(maxFeeds), itsMaxFields(maxFields),
      itsPointingTolerance(pointingTol), itsFreqDep(frequencyDependent)
      
    {	
      ASKAPCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
      ASKAPCHECK(maxFields>0, "Maximum number of fields must be one or more");
      ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
      ASKAPCHECK(maxSupport>0, "Maximum support must be greater than 0")
	ASKAPCHECK(pointingTol>0.0, "Pointing tolerance must be greater than 0.0");
      ASKAPDEBUGASSERT(itsIllumination);
      itsSupport=0;
      itsOverSample=overSample;
      itsMaxSupport=maxSupport;
      itsLimitSupport=limitSupport;
      itsName=name;
      
      itsSlopes.resize(2, itsMaxFeeds, itsMaxFields);
      itsSlopes.set(0.0);
      itsDone.resize(itsMaxFeeds, itsMaxFields);
      itsDone.set(false);
      itsPointings.resize(itsMaxFeeds, itsMaxFields);
      itsPointings.set(casa::MVDirection());
      itsLastField=-1;
      itsCurrentField=0;
    }
    
    /// @brief copy constructor
    /// @details It is required to decouple internal arrays between input object
    /// and this copy.
    /// @param[in] other input object
    /// @note illumination pattern is copied as a shared pointer, hence referencing
    /// the same model
    AProjectWStackVisGridder::AProjectWStackVisGridder(const AProjectWStackVisGridder &other) :
      WStackVisGridder(other), itsReferenceFrequency(other.itsReferenceFrequency),
      itsIllumination(other.itsIllumination), itsMaxFeeds(other.itsMaxFeeds),
      itsMaxFields(other.itsMaxFields), itsPointingTolerance(other.itsPointingTolerance),
      itsLastField(other.itsLastField), itsCurrentField(other.itsCurrentField),
      itsFreqDep(other.itsFreqDep), itsMaxSupport(other.itsMaxSupport),
      itsLimitSupport(other.itsLimitSupport),
      itsCMap(other.itsCMap.copy()), itsSlopes(other.itsSlopes.copy()),
      itsDone(other.itsDone.copy()), itsPointings(other.itsPointings.copy())
    {
    }
    
    AProjectWStackVisGridder::~AProjectWStackVisGridder() {
         size_t nUsed = 0;
         for (casa::uInt feed = 0; feed<itsDone.nrow(); ++feed) {
              for (casa::uInt field = 0; field<itsDone.ncolumn(); ++field) {
                   if (itsDone(feed,field)) {
                       ++nUsed;
                   }
              }
         }
         if (itsDone.nelements()) {
             ASKAPLOG_INFO_STR(logger, "AProjectWStackVisGridder cache usage: "<<
                         double(nUsed)/double(itsDone.nrow()*itsDone.ncolumn())*100<<"% of maxfeed*maxfield");
         }    
    }
    
    /// Clone a copy of this Gridder
    IVisGridder::ShPtr AProjectWStackVisGridder::clone() {
      return IVisGridder::ShPtr(new AProjectWStackVisGridder(*this));
    }
    
    /// Initialize the indices into the cube.
    void AProjectWStackVisGridder::initIndices(const IConstDataAccessor& acc) {
      
      // Validate cache using first row only
      bool newField=true;
      
      int firstFeed=acc.feed1()(0);
      ASKAPCHECK(firstFeed<itsMaxFeeds, "Too many feeds: increase maxfeeds");
      casa::MVDirection firstPointing=acc.pointingDir1()(0);
      
      for (int field=itsLastField; field>-1; --field) {
	if (firstPointing.separation(itsPointings(firstFeed, field))
	    <itsPointingTolerance) {
	  itsCurrentField=field;
	  newField=false;
	  break;
	}
      }
      if (newField) {
	itsLastField++;
	itsCurrentField=itsLastField;
	ASKAPCHECK(itsCurrentField<itsMaxFields,
		   "Too many fields: increase maxfields " << itsMaxFields);
	itsPointings(firstFeed, itsCurrentField)=firstPointing;
	ASKAPLOG_INFO_STR(logger, "Found new field " << itsCurrentField<<" at "<<
			  printDirection(firstPointing));
      }
      
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = acc.nRow();
      const int nChan = acc.nChannel();
      
      const int nPol = acc.nPol();
      itsCMap.resize(nSamples, nPol, nChan);
      itsCMap.set(0);
      /// @todo Select max feeds more carefully
      
      itsGMap.resize(nSamples, nPol, nChan);
      itsGMap.set(0);
      
      const int cenw=(itsNWPlanes-1)/2;
      
      for (int i=0; i<nSamples; ++i) {
	const int feed=acc.feed1()(i);
	ASKAPCHECK(feed<itsMaxFeeds,
		   "Exceeded specified maximum number of feeds");
	ASKAPCHECK(feed>-1, "Illegal negative feed number");
	
	const double w=(acc.uvw()(i)(2))/(casa::C::c);
	
	for (int chan=0; chan<nChan; ++chan) {
	  const double freq=acc.frequency()[chan];
	  for (int pol=0; pol<nPol; pol++) {
	    /// Order is (chan, feed)
	    if(itsFreqDep) {
	      itsCMap(i, pol, chan)=chan+nChan*(feed+itsMaxFeeds*itsCurrentField);
	      ASKAPCHECK(itsCMap(i, pol, chan)<itsMaxFields*itsMaxFeeds
			 *nChan, "CMap index too large");
	      ASKAPCHECK(itsCMap(i, pol, chan)>-1,
			 "CMap index less than zero");
	    }
	    else {
	      itsCMap(i, pol, chan)=(feed+itsMaxFeeds*itsCurrentField);
	      ASKAPCHECK(itsCMap(i, pol, chan)<itsMaxFields*itsMaxFeeds*nChan,
			 "CMap index too large");
	      ASKAPCHECK(itsCMap(i, pol, chan)>-1, "CMap index less than zero");
	    }
	    
	    /// Calculate the index into the grids
	    if (itsNWPlanes>1) {
	      itsGMap(i, pol, chan)=cenw+nint(w*freq/itsWScale);
	    } else {
	      itsGMap(i, pol, chan)=0;
	    }
	    ASKAPCHECK(itsGMap(i, pol, chan)<itsNWPlanes,
		       "W scaling error: recommend allowing larger range of w");
	    ASKAPCHECK(itsGMap(i, pol, chan)>-1,
		       "W scaling error: recommend allowing larger range of w");
	  }
	}
      }
    }
    
    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    /// @todo Make initConvolutionFunction more robust
    void AProjectWStackVisGridder::initConvolutionFunction(const IConstDataAccessor& acc) {
      
      ASKAPDEBUGASSERT(itsIllumination);
      // just to avoid a repeated call to a virtual function from inside the loop
      const bool hasSymmetricIllumination = itsIllumination->isSymmetric();
      
      casa::MVDirection out = getImageCentre();
      const int nSamples = acc.nRow();
      
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nChan = itsFreqDep ? acc.nChannel() : 1;
      
      if(itsSupport==0) {
	ASKAPLOG_INFO_STR(logger, "Resizing convolution function to "
			  << itsOverSample*itsOverSample*itsMaxFeeds*itsMaxFields*nChan << " entries");
	itsConvFunc.resize(itsOverSample*itsOverSample*itsMaxFeeds*itsMaxFields*nChan);

	ASKAPLOG_INFO_STR(logger, "Resizing sum of weights to "
			  << itsMaxFeeds*itsMaxFields*nChan << " entries");
	itsSumWeights.resize(itsMaxFeeds*itsMaxFields*nChan, itsShape(2), itsShape(3));
	itsSumWeights.set(0.0);
      }
      
      /// Limit the size of the convolution function since
      /// we don't need it finely sampled in image space. This
      /// will reduce the time taken to calculate it.
      casa::uInt nx=std::min(itsMaxSupport, itsShape(0));
      casa::uInt ny=std::min(itsMaxSupport, itsShape(1));
      
      // this is just a buffer in the uv-space
      UVPattern pattern(nx,ny, itsUVCellSize(0),itsUVCellSize(1),itsOverSample);
      
      int nDone=0;
      for (int row=0; row<nSamples; ++row) {
	const int feed=acc.feed1()(row);
	
	if (!itsDone(feed, itsCurrentField)) {
	  itsDone(feed, itsCurrentField)=true;
	  nDone++;
	  casa::MVDirection offset(acc.pointingDir1()(row).getAngle());
	  itsSlopes(0, feed, itsCurrentField) = isPSFGridder() ? 0. : sin(offset.getLong()
						   -out.getLong()) *cos(offset.getLat());
	  itsSlopes(1, feed, itsCurrentField)= isPSFGridder() ? 0. : sin(offset.getLat())
	    *cos(out.getLat()) - cos(offset.getLat())*sin(out.getLat())
	    *cos(offset.getLong()-out.getLong());
	  
	  const double parallacticAngle = hasSymmetricIllumination ? 0. : acc.feed1PA()(row);
	  
	  for (int chan=0; chan<nChan; chan++) {
	    /// Extract illumination pattern for this channel
	    itsIllumination->getPattern(acc.frequency()[chan], pattern,
					itsSlopes(0, feed, itsCurrentField),
					itsSlopes(1, feed, itsCurrentField), 
					parallacticAngle);
	    
	    /// Now convolve the disk with itself using an FFT
	    fft2d(pattern.pattern(), false);
	    
	    double peak=0.0;
	    for (casa::uInt ix=0; ix<nx; ++ix) {
	      for (casa::uInt iy=0; iy<ny; ++iy) {
		pattern(ix, iy)=pattern(ix,iy)*conj(pattern(ix,iy));
		if(casa::abs(pattern(ix,iy))>peak) peak=casa::abs(pattern(ix,iy));
	      }
	    }
	    if(peak>0.0) {
	      pattern.pattern()*=casa::Complex(1.0/peak);
	    }
	    // The maximum will be 1.0
	    //	    ASKAPLOG_INFO_STR(logger, "Max of FT of convolution function = " << casa::max(pattern.pattern()));
	    fft2d(pattern.pattern(), true);	
	    // Now correct for normalization of FFT
	    pattern.pattern()*=casa::Complex(1.0/(double(nx)*double(ny)));
	    
	    if (itsSupport==0) {
	      // we probably need a proper support search here
	      // it can be encapsulated in a method of the UVPattern class
	      itsSupport = pattern.maxSupport();
	      ASKAPCHECK(itsSupport>0,
			 "Unable to determine support of convolution function");
	      ASKAPCHECK(itsSupport*itsOverSample<int(nx)/2,
			 "Overflowing convolution function - increase maxSupport or decrease overSample");
	      if (itsLimitSupport > 0  &&  itsSupport > itsLimitSupport) {
		ASKAPLOG_INFO_STR(logger, "Convolution function support = "
				  << itsSupport << " pixels exceeds upper support limit; "
				  << "set to limit = " << itsLimitSupport << " pixels");
		itsSupport = itsLimitSupport;
	      }
	      itsCSize=2*itsSupport+1;
	      // just for logging
	      const double cell = std::abs(pattern.uCellSize())*(casa::C::c/acc.frequency()[chan]);
	      ASKAPLOG_INFO_STR(logger, "Convolution function support = "
				<< itsSupport << " pixels, size = " << itsCSize
				<< " pixels");
	      ASKAPLOG_INFO_STR(logger, "Maximum extent = "<< itsSupport
				*cell << " (m) sampled at "<< cell
				<< " (m)");
	      ASKAPLOG_INFO_STR(logger, "Number of planes in convolution function = "
				<< itsConvFunc.size());
	    } // if itsSupport uninitialized
	    int zIndex=chan+nChan*(feed+itsMaxFeeds*itsCurrentField);
	    
	    // Since we are decimating, we need to rescale by the
	    // decimation factor
	    float rescale=float(itsOverSample*itsOverSample);
	    for (int fracu=0; fracu<itsOverSample; fracu++) {
	      for (int fracv=0; fracv<itsOverSample; fracv++) {
		int plane=fracu+itsOverSample*(fracv+itsOverSample*zIndex);
		ASKAPDEBUGASSERT(plane>=0 && plane<int(itsConvFunc.size()));
		itsConvFunc[plane].resize(itsCSize, itsCSize);
		itsConvFunc[plane].set(0.0);
		// Now cut out the inner part of the convolution function and
		// insert it into the cache
		for (int iy=-itsSupport; iy<itsSupport; iy++) {
		  for (int ix=-itsSupport; ix<itsSupport; ix++) {
		    itsConvFunc[plane](ix+itsSupport, iy+itsSupport)
		      = rescale * pattern(itsOverSample*ix+fracu+nx/2,
				itsOverSample*iy+fracv+ny/2);
		  } // for ix
		} // for iy
		//
		//ASKAPLOG_INFO_STR(logger, "convolution function for channel "<<chan<<
		//   " plane="<<plane<<" has an integral of "<<sum(itsConvFunc[plane]));						
		//
	      } // for fracv
	    } // for fracu								
	  } // for chan
	} // if !isDone
      } // for row
      
      ASKAPCHECK(itsSupport>0, "Support not calculated correctly");
      
    }
    
    // To finalize the transform of the weights, we use the following steps:
    // 1. For each plane of the convolution function, transform to image plane
    // and multiply by conjugate to get abs value squared.
    // 2. Sum all planes weighted by the weight for that convolution function.
    void AProjectWStackVisGridder::finaliseWeights(casa::Array<double>& out) {
      
      ASKAPLOG_INFO_STR(logger, "Calculating sum of weights image");
      ASKAPDEBUGASSERT(itsShape.nelements()>=3);
      
      const int nx=itsShape(0);
      const int ny=itsShape(1);
      const int nPol=itsShape(2);
      const int nChan=itsShape(3);
      
      const int nZ=itsSumWeights.shape()(0);
      
      /// We must pad the convolution function to full size, reverse transform
      /// square, and sum multiplied by the corresponding weight
      const int cnx=std::min(itsMaxSupport, nx);
      const int cny=std::min(itsMaxSupport, ny);
      const int ccenx = cnx/2;
      const int cceny = cny/2;
      
      /// This is the output array before sinc padding
      casa::Array<double> cOut(casa::IPosition(4, cnx, cny, nPol, nChan));
      cOut.set(0.0);
      
      // for debugging
      double totSumWt = 0.;
      
      /// itsSumWeights has one element for each separate data plane (feed, field, chan)
      /// itsConvFunc has overSampling**2 planes for each separate data plane (feed, field, chan)
      /// We choose the convolution function at zero fractional offset in u,v 
      for (int iz=0; iz<nZ; ++iz) {
           const int plane=itsOverSample*itsOverSample*iz;
	
           bool hasData=false;
	       for (int chan=0; chan<nChan; ++chan) {
	            for (int pol=0; pol<nPol; ++pol) {
                     const double wt = itsSumWeights(iz, pol, chan);
	                 if (wt > 0.0) {
                         hasData=true;
                         totSumWt += wt;
                         //break;
                     }
                }
           }
	if(hasData) {
	  
	  // Now fill the inner part of the uv plane with the convolution function
	  // and transform to obtain the image. The uv sampling is fixed here
	  // so the total field of view is itsOverSample times larger than the
	  // original field of view.
	  /// Work space
	  casa::Matrix<casa::Complex> thisPlane(cnx, cny);
	  thisPlane.set(0.0);
	  for (int iy=-itsSupport; iy<+itsSupport; ++iy) {
	    for (int ix=-itsSupport; ix<+itsSupport; ++ix) {
	      thisPlane(ix+ccenx, iy+cceny)=itsConvFunc[plane](ix+itsSupport, iy+itsSupport);
	    }
	  }
	  
	  //	  	  ASKAPLOG_INFO_STR(logger, "Convolution function["<< iz << "] peak = "<< peak);
	  fft2d(thisPlane, false);
	  thisPlane*=casa::Complex(nx*ny);
	  float peak=real(casa::max(casa::abs(thisPlane)));
	  //	  ASKAPLOG_INFO_STR(logger, "Transform of convolution function["<< iz
	  //			    << "] peak = "<< peak);

	  if(peak>0.0) {
	    thisPlane*=casa::Complex(1.0/peak);
	  }
	  
	  // Now we need to cut out only the part inside the field of view
	  for (int chan=0; chan<nChan; ++chan) {
	    for (int pol=0; pol<nPol; ++pol) {
	      casa::IPosition ip(4, 0, 0, pol, chan);
	      const double wt=itsSumWeights(iz, pol, chan);
	      for (int ix=0; ix<cnx; ++ix) {
		ip(0)=ix;
		for (int iy=0; iy<cny; ++iy) {
		  ip(1)=iy;
		  cOut(ip)+=wt*casa::real(thisPlane(ix, iy)*conj(thisPlane(ix, iy)));
		}
	      }
	    }
	  }
	} // if has data
      } // loop over convolution functions
      
      fftPad(cOut, out);
      ASKAPLOG_INFO_STR(logger, 
			"Finished finalising the weights, the sum over all convolution functions is "<<totSumWt);	
    }
    
    void AProjectWStackVisGridder::fftPad(const casa::Array<double>& in,
					  casa::Array<double>& out) {
      
      int inx=in.shape()(0);
      int iny=in.shape()(1);
      
      int onx=out.shape()(0);
      int ony=out.shape()(1);
      
      // Shortcut no-op
      if ((inx==onx)&&(iny==ony)) {
          out=in.copy();
          return;
      }
      
      ASKAPCHECK((onx>=inx)==(ony>=iny), "Attempting to pad to a rectangular array smaller on one axis");
      if (onx<inx) {
          // no fft padding required, the output array is smaller.
          casa::Array<double> tempIn(in); // in is a conceptual const array here
          out = PaddingUtils::centeredSubArray(tempIn,out.shape()).copy();
          return;
      }
      
      /// Make an iterator that returns plane by plane
      casa::ReadOnlyArrayIterator<double> inIt(in, 2);
      casa::ArrayIterator<double> outIt(out, 2);
      while (!inIt.pastEnd()&&!outIt.pastEnd()) {
	casa::Matrix<casa::DComplex> inPlane(inx, iny);
	casa::Matrix<casa::DComplex> outPlane(onx, ony);
	casa::convertArray(inPlane, inIt.array());
	outPlane.set(0.0);
	fft2d(inPlane, false);
	for (int iy=0; iy<iny; iy++) {
	  for (int ix=0; ix<inx; ix++) {
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
    
    int AProjectWStackVisGridder::cIndex(int row, int pol, int chan) {
      return itsCMap(row, pol, chan);
    }
    
    void AProjectWStackVisGridder::correctConvolution(casa::Array<double>& grid) {
    }
    
  }
}

