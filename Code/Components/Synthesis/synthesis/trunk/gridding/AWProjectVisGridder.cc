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

#include <measurementequation/SynthesisParamsHelper.h>

#include <gridding/AWProjectVisGridder.h>

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

#include <gridding/SupportSearcher.h>

#include <fft/FFTWrapper.h>

#include <casa/Arrays/ArrayIter.h>

using namespace askap;

namespace askap {
  namespace synthesis {
    
    AWProjectVisGridder::AWProjectVisGridder(const boost::shared_ptr<IBasicIllumination const> &illum,
					     const double wmax, const int nwplanes,
					     const double cutoff, const int overSample,
					     const int maxSupport, const int limitSupport,
					     const int maxFeeds, const int maxFields, const double pointingTol,
					     const bool frequencyDependent, const std::string& name) :
      WProjectVisGridder(wmax, nwplanes, cutoff, overSample, maxSupport, limitSupport, name),
      itsReferenceFrequency(0.0), itsIllumination(illum),
      itsFreqDep(frequencyDependent),
      itsMaxFeeds(maxFeeds), itsMaxFields(maxFields),
      itsPointingTolerance(pointingTol)
      
    {
      ASKAPDEBUGASSERT(itsIllumination);
      ASKAPCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
      ASKAPCHECK(maxFields>0, "Maximum number of fields must be one or more");
      ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
      ASKAPCHECK(maxSupport>0, "Maximum support must be greater than 0")
	ASKAPCHECK(pointingTol>0.0, "Pointing tolerance must be greater than 0.0");
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
    /// @details It is required to decouple internal array arrays, otherwise
    /// those arrays are shared between all cloned gridders of this type
    /// @note illumination model is copied as a pointer, so the same model is referenced
    /// @param[in] other input object
    AWProjectVisGridder::AWProjectVisGridder(const AWProjectVisGridder &other) :
      WProjectVisGridder(other), itsReferenceFrequency(other.itsReferenceFrequency),
      itsIllumination(other.itsIllumination), itsFreqDep(other.itsFreqDep),
      itsMaxFeeds(other.itsMaxFeeds), itsMaxFields(other.itsMaxFields),
      itsPointingTolerance(other.itsPointingTolerance), 
      itsCurrentField(other.itsCurrentField), itsLastField(other.itsLastField),
      itsPointings(other.itsPointings.copy()), itsSlopes(other.itsSlopes.copy()),
      itsDone(other.itsDone.copy()) {}
    
    AWProjectVisGridder::~AWProjectVisGridder() {
    }
    
    /// Clone a copy of this Gridder
    IVisGridder::ShPtr AWProjectVisGridder::clone() {
      return IVisGridder::ShPtr(new AWProjectVisGridder(*this));
    }
    
    /// Initialize the indices into the cube.
    void AWProjectVisGridder::initIndices(const IConstDataAccessor& acc) {
      
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
	ASKAPLOG_INFO_STR(logger, "Found new field " << itsCurrentField);
      }
      
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = acc.nRow();
      const int nChan = acc.nChannel();
      
      const int nPol = acc.nPol();
      itsCMap.resize(nSamples, nPol, nChan);
      itsCMap.set(0);
      
      const int cenw=(itsNWPlanes-1)/2;
      
      for (int i=0; i<nSamples; ++i) {
	const int feed=acc.feed1()(i);
	ASKAPCHECK(feed<itsMaxFeeds,
		   "Exceeded specified maximum number of feeds");
	ASKAPCHECK(feed>-1, "Illegal negative feed number");
	
	const double w=(acc.uvw()(i)(2))/(casa::C::c);
	
	for (int chan=0; chan<nChan; ++chan) {
	  const double freq=acc.frequency()[chan];
	  int iw=0;
	  if (itsNWPlanes>1) {
	    iw=cenw+int(w*freq/itsWScale);
	  }
	  ASKAPCHECK(iw<itsNWPlanes,
		     "W scaling error: recommend allowing larger range of w");
	  ASKAPCHECK(iw>-1,
		     "W scaling error: recommend allowing larger range of w");
	  
	  for (int pol=0; pol<nPol; ++pol) {
	    /// Order is (iw, chan, feed)
	    if (itsFreqDep) {
	      itsCMap(i, pol, chan)=iw+itsNWPlanes*(chan+nChan*(feed+itsMaxFeeds*itsCurrentField));
	      ASKAPCHECK(itsCMap(i, pol, chan)<itsNWPlanes*itsMaxFeeds
			 *itsMaxFields*nChan, "CMap index too large");
	      ASKAPCHECK(itsCMap(i, pol, chan)>-1,
			 "CMap index less than zero");
	    } else {
	      itsCMap(i, pol, chan)=iw+itsNWPlanes*(feed+itsMaxFeeds*itsCurrentField);
	      ASKAPCHECK(itsCMap(i, pol, chan)<itsNWPlanes*itsMaxFeeds
			 *itsMaxFields, "CMap index too large");
	      ASKAPCHECK(itsCMap(i, pol, chan)>-1,
			 "CMap index less than zero");
	    }
	  }
	}
      }
    }
    
    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    /// @todo Make initConvolutionFunction more robust
    void AWProjectVisGridder::initConvolutionFunction(const IConstDataAccessor& acc) {
      casa::MVDirection out = getImageCentre();
      const int nSamples = acc.nRow();
      
      ASKAPDEBUGASSERT(itsIllumination);
      // just to avoid a repeated call to a virtual function from inside the loop
      const bool hasSymmetricIllumination = itsIllumination->isSymmetric();
      
      
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nChan=itsFreqDep ? acc.nChannel() : 1;
      
      if(itsSupport==0) {
	itsConvFunc.resize(itsOverSample*itsOverSample*itsNWPlanes*itsMaxFeeds*itsMaxFields*nChan);
	itsSumWeights.resize(itsNWPlanes*itsMaxFeeds*itsMaxFields*nChan, itsShape(2), itsShape(3));
	itsSumWeights.set(0.0);
      }
      
      const int cenw=(itsNWPlanes-1)/2;
      
      /// Limit the size of the convolution function since
      /// we don't need it finely sampled in image space. This
      /// will reduce the time taken to calculate it.
      casa::uInt nx=std::min(itsMaxSupport, itsShape(0));
      casa::uInt ny=std::min(itsMaxSupport, itsShape(1));
      
      casa::uInt qnx=nx/itsOverSample;
      casa::uInt qny=ny/itsOverSample;
      
      // Find the actual cellsizes in x and y (radians) 
      // corresponding to the limited support
      double ccellx=1.0/(double(qnx)*itsUVCellSize(0));
      double ccelly=1.0/(double(qny)*itsUVCellSize(1));

      casa::Vector<float> ccfx(nx);
      casa::Vector<float> ccfy(ny);
      for (casa::uInt ix=0; ix<nx; ++ix) {
           const float nux=std::abs(float(ix)-float(nx/2))/float(nx/2);
           ccfx(ix)=grdsf(nux); // /float(qnx);
      }
      for (casa::uInt iy=0; iy<ny; ++iy) {
           const float nuy=std::abs(float(iy)-float(ny/2))/float(ny/2);
           ccfy(iy)=grdsf(nuy); // /float(qny);
      }
      
      // this is just a buffer in the uv-space, oversampling is
      // taken into account inside the UVPattern object (in the past we handled
      // oversampling explicitly by using qnx and qny instead of nx and ny and
      // passing 1 instead of itsOverSample, but it caused scaling problems for
      // offset feeds).
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
	  
	  for (int chan=0; chan<nChan; ++chan) {
	    
	    /// Extract illumination pattern for this channel
	    itsIllumination->getPattern(acc.frequency()[chan], pattern,
					itsSlopes(0, feed, itsCurrentField),
					itsSlopes(1, feed, itsCurrentField), parallacticAngle);
	    
	    fft2d(pattern.pattern(), false);
	    	    
	    
	    /// Calculate the total convolution function including
	    /// the w term and the antenna convolution function
	    casa::Matrix<casa::Complex> thisPlane(nx, ny);
	    	    
	    for (int iw=0; iw<itsNWPlanes; ++iw) {
	      thisPlane.set(0.0);
	      
	      
	      // Loop over the central nx, ny region, setting it to the product
	      // of the phase screen and the spheroidal function
	      double maxCF=0.0;
	      double peak=0.0;
	      double w=2.0f*casa::C::pi*double(iw-cenw)*itsWScale;
	      //std::cout<<"plane "<<iw<<" w="<<w<<std::endl;
	      
	      for (int iy=0; iy<int(ny); ++iy) {
               double y2=(double(iy)-double(ny)/2)*ccelly;
               y2*=y2;
               for (int ix=0; ix<int(nx); ++ix) {
                    double x2=(double(ix)-double(nx)/2)*ccellx;
                    x2*=x2;
                    double r2=x2+y2;
                    if (r2<1.0) {
                        const double phase=w*(1.0-sqrt(1.0-r2));
                        const casa::Complex wt=pattern(ix, iy)*conj(pattern(ix, iy))
                                       *casa::Complex(ccfx(ix)*ccfy(iy));
                        if(casa::abs(wt)>peak) {
                           peak=casa::abs(wt);
                        }
                        // this ensures the oversampling is done
                        thisPlane(ix, iy)=wt*casa::Complex(cos(phase), -sin(phase));
                        maxCF+=casa::abs(wt);
                    }
		       }
	      }	
	      
	      	      

	      ASKAPCHECK(maxCF>0.0, "Convolution function is empty");
	      thisPlane*=casa::Complex(1.0/peak);
	      maxCF/=peak;
	      
	      
	      // At this point, we have the phase screen multiplied by the spheroidal
	      // function, sampled on larger cellsize (itsOverSample larger) in image
	      // space. Only the inner qnx, qny pixels have a non-zero value
	      	      
	      // Now we have to calculate the Fourier transform to get the
	      // convolution function in uv space
	      fft2d(thisPlane, true);
	      
	      // Now correct for normalization of FFT
	      thisPlane*=casa::Complex(1.0/(double(nx)*double(ny)));
	      maxCF/=double(nx)*double(ny);
	      	      	    
	      // If the support is not yet set, find it and size the
	      // convolution function appropriately
	      if (itsSupport==0) {	
              //  SynthesisParamsHelper::saveAsCasaImage("dbg.img", amplitude(thisPlane));
              SupportSearcher ss(itsCutoff);
              ss.search(thisPlane);
              
              
              itsSupport = ss.symmetricalSupport(thisPlane.shape())/2/itsOverSample;
              //std::cout<<itsSupport<<" "<<nx<<" "<<itsCutoff<<" "<<itsOverSample<<std::endl;

              ASKAPCHECK(itsSupport>0,
                         "Unable to determine support of convolution function");
              ASKAPCHECK(itsSupport*itsOverSample<int(nx)/2,
                         "Overflowing convolution function - increase maxSupport or decrease overSample")
        
        //SynthesisParamsHelper::saveAsCasaImage("dbg.img", amplitude(thisPlane));
	    //throw 1;
        	   
		  if (itsLimitSupport > 0  &&  itsSupport > itsLimitSupport) {
		    ASKAPLOG_INFO_STR(logger, "Convolution function support = "
				      << itsSupport << " pixels exceeds upper support limit; "
				      << "set to limit = " << itsLimitSupport << " pixels");
		    itsSupport = itsLimitSupport;
		  }
		itsCSize=2*itsSupport+1;
		ASKAPLOG_INFO_STR(
				  logger,
				  "Convolution function support = " << itsSupport
				  << " pixels, convolution function size = "
				  << itsCSize << " pixels");
		// just for log output					
		const double cell=std::abs(itsUVCellSize(0)*(casa::C::c
							     /acc.frequency()[chan]));
		ASKAPLOG_INFO_STR(logger, "Maximum extent = "
				  << itsSupport*cell << " (m) sampled at "<< cell
				  /itsOverSample << " (m)");
		itsCCenter=itsSupport;
		ASKAPLOG_INFO_STR(logger, "Number of planes in convolution function = "
				  << itsConvFunc.size());
	      }
	      int zIndex=iw+itsNWPlanes*(chan+nChan*(feed+itsMaxFeeds*itsCurrentField));
	      
	      // Since we are decimating, we need to rescale by the
	      // decimation factor
	      float rescale=float(itsOverSample*itsOverSample);
	      for (int fracu=0; fracu<itsOverSample; fracu++) {
		for (int fracv=0; fracv<itsOverSample; fracv++) {
		  int plane=fracu+itsOverSample*(fracv+itsOverSample
						 *zIndex);
		  ASKAPDEBUGASSERT(plane>=0 && plane<int(itsConvFunc.size()));
		  itsConvFunc[plane].resize(itsCSize, itsCSize);
		  itsConvFunc[plane].set(0.0);
		  // Now cut out the inner part of the convolution function and
		  // insert it into the convolution function
		  for (int iy=-itsSupport; iy<itsSupport; iy++) {
		    for (int ix=-itsSupport; ix<itsSupport; ix++) {
		      itsConvFunc[plane](ix+itsCCenter, iy+itsCCenter)
			= rescale*thisPlane(ix*itsOverSample+fracu+nx/2,
					    iy*itsOverSample+fracv+ny/2);
		    } // for ix
		  } // for iy
		   		  
		} // for fracv
	      } // for fracu
	    } // w loop
	  } // chan loop
	}
      }
      
      
      if (nDone == itsMaxFeeds*itsMaxFields*itsNWPlanes) {
	ASKAPLOG_INFO_STR(logger, "Shape of convolution function = "
			  << itsConvFunc[0].shape() << " by "<< itsConvFunc.size()
			  << " planes");
      }
      
      ASKAPCHECK(itsSupport>0, "Support not calculated correctly");
    }
    
    
    
    /// To finalize the transform of the weights, we use the following steps:
    /// 1. For each plane of the convolution function, transform to image plane
    /// and multiply by conjugate to get abs value squared.
    /// 2. Sum all planes weighted by the weight for that convolution function.
    void AWProjectVisGridder::finaliseWeights(casa::Array<double>& out) {
      
      ASKAPLOG_INFO_STR(logger, "Calculating sum of weights image");
      
      const int nx=itsShape(0);
      const int ny=itsShape(1);
      const int nPol=itsShape(2);
      const int nChan=itsShape(3);
      
      const int nZ=itsSumWeights.shape()(0);
      
      /// We must pad the convolution function to full size, reverse transform
      /// square, and sum multiplied by the corresponding weight
      const int cnx=std::min(itsMaxSupport, nx);
      const int cny=std::min(itsMaxSupport, ny);
      const int ccenx=cnx/2;
      const int cceny=cny/2;
      
      /// This is the output array before sinc padding
      casa::Array<double> cOut(casa::IPosition(4, cnx, cny, nPol, nChan));
      cOut.set(0.0);
      
      // for debugging
      double totSumWt = 0.;
      
      /// itsSumWeights has one element for each separate data plane (feed, field, chan)
      /// itsConvFunc has overSampling**2 planes for each separate data plane (feed, field, chan)
      /// We choose the convolution function at zero fractional offset in u,v 
      for (int iz=0; iz<nZ; iz++) {
	const int plane=itsOverSample*itsOverSample*iz;
	
	bool hasData=false;
	for (int chan=0; chan<nChan; chan++) {
	  for (int pol=0; pol<nPol; pol++) {
	    double wt=itsSumWeights(iz, pol, chan);
	    if(wt>0.0) {
	      hasData=true;
	      totSumWt += wt;
	      //	      break;
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
	  ASKAPDEBUGASSERT(itsCCenter == itsSupport);
	  for (int iy=-itsSupport; iy<+itsSupport; iy++) {
	    for (int ix=-itsSupport; ix<+itsSupport; ix++) {
	      thisPlane(ix+ccenx, iy+cceny)=itsConvFunc[plane](ix+itsCCenter, iy+itsCCenter);
	    }
	  }	  
	  
	  float peak=real(casa::max(casa::abs(thisPlane)));
	  //	  ASKAPLOG_INFO_STR(logger, "Convolution function["<< iz << "] peak = "<< peak);
	  fft2d(thisPlane, false);
	  thisPlane*=casa::Complex(nx*ny);
	  
	  peak=real(casa::max(casa::abs(thisPlane)));
	  //	  ASKAPLOG_INFO_STR(logger, "Transform of convolution function["<< iz
	  //			    << "] peak = "<< peak);
	  if(peak>0.0) {
	    //	    thisPlane*=casa::Complex(float(nx*ny)/peak);
	    thisPlane*=casa::Complex(1.0/peak);
	  }
	  
	  
	  // Now we need to cut out only the part inside the field of view
	  for (int chan=0; chan<nChan; chan++) {
	    for (int pol=0; pol<nPol; pol++) {
	      casa::IPosition ip(4, 0, 0, pol, chan);
	      double wt=itsSumWeights(iz, pol, chan);
	      for (int ix=0; ix<cnx; ix++) {
		ip(0)=ix;
		for (int iy=0; iy<cny; iy++) {
		  ip(1)=iy;
		  cOut(ip)+=float(wt)*casa::real(thisPlane(ix, iy)
						 *conj(thisPlane(ix, iy)));
		}
	      }
	    }
	  }
	}
      }
      
      fftPad(cOut, out);
 
      ASKAPLOG_INFO_STR(logger, 
			"Finished finalising the weights, the sum over all convolution functions is "<<totSumWt);	
    }

/*
// this is an experimental version in Max's tree. Commented out for a check in
    /// This is the default implementation
void AWProjectVisGridder::finaliseGrid(casa::Array<double>& out) {
   
   ASKAPDEBUGASSERT(isPSFGridder());
   // we need to do all calculations on a limited support (e.g. because the convolution correction is
   // done for the image of that size)
   casa::IPosition limitedSupportShape(out.shape());
   ASKAPDEBUGASSERT(limitedSupportShape.nelements()>=2);
   limitedSupportShape(0) = std::min(itsMaxSupport, limitedSupportShape(0));
   limitedSupportShape(1) = std::min(itsMaxSupport, limitedSupportShape(1));
   // a buffer, which will be padded to the full size at the end
   casa::Array<double> cOut(limitedSupportShape);

   /// Loop over all grids Fourier transforming and accumulating
   for (unsigned int i=0; i<itsGrid.size(); i++) {
		casa::Array<casa::Complex> scratch(itsGrid[i].copy());
        std::cout<<itsGrid[i].shape()<<std::endl;
        
		fft2d(scratch, false);
		
        if (i==0) {
            toDouble(cOut, scratch);
        } else {
            casa::Array<double> work(out.shape());
            toDouble(work, scratch);
            cOut+=work;
        }
    }
    // Now we can do the convolution correction    
    correctConvolution(cOut);
    cOut*=double(cOut.shape()(0))*double(cOut.shape()(1)); 
    
    casa::Array<float> test(cOut.shape());
    convertArray(test,cOut);
    SynthesisParamsHelper::saveAsCasaImage("dbg.img", test);
	throw 1;
    fftPad(cOut,out);
}
*/    

/// Correct for gridding convolution function
/// @param image image to be corrected
void AWProjectVisGridder::correctConvolution(casa::Array<double>& image)
{
   /*
     ASKAPDEBUGASSERT(itsShape.nelements()>=2);      
     
     casa::IPosition limitedSupportShape(image.shape());
     ASKAPDEBUGASSERT(limitedSupportShape.nelements()>=2);
        
      const casa::Int xHalfSize = limitedSupportShape(0)/2;
      const casa::Int yHalfSize = limitedSupportShape(1)/2;
      
      
      casa::Vector<double> ccfx(limitedSupportShape(0));
      casa::Vector<double> ccfy(limitedSupportShape(1));
      for (int ix=0; ix<limitedSupportShape(0); ++ix)
      {
        const double nux=std::abs(double(ix-xHalfSize))/double(xHalfSize*itsOverSample);
        ccfx(ix)=1.0/grdsf(nux);
      }
      for (int iy=0; iy<limitedSupportShape(1); ++iy)
      {
        const double nuy=std::abs(double(iy-yHalfSize))/double(yHalfSize*itsOverSample);
        ccfy(iy)=1.0/grdsf(nuy);
      }

      casa::ArrayIterator<double> it(image, 2);
      while (!it.pastEnd())
      {
        casa::Matrix<double> mat(it.array());
        for (int ix=0; ix<limitedSupportShape(0); ix++)
        {
          for (int iy=0; iy<limitedSupportShape(1); iy++)
          {
            mat(ix, iy)*=ccfx(ix)*ccfy(iy);
          }
        }
        it.next();
      }
      */     
      /*   
      casa::Array<float> test(image.shape());
      convertArray(test,image);
      SynthesisParamsHelper::saveAsCasaImage("dbg.img", test);
	  throw 1;
*/
}
    
    void AWProjectVisGridder::fftPad(const casa::Array<double>& in,
				     casa::Array<double>& out) {
      
      const int inx=in.shape()(0);
      const int iny=in.shape()(1);
      
      const int onx=out.shape()(0);
      const int ony=out.shape()(1);
            
      // Shortcut no-op
      if ((inx==onx)&&(iny==ony)) {
	out=in.copy();
	return;
      }
      
      ASKAPCHECK(onx>=inx, "Attempting to pad to smaller array");
      ASKAPCHECK(ony>=iny, "Attempting to pad to smaller array");
      
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
    
    int AWProjectVisGridder::cIndex(int row, int pol, int chan) {
      return itsCMap(row, pol, chan);
    }
    
    
  }
}
