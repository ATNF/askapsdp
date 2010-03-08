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
#include <utils/PaddingUtils.h>

// we may need to move this include into AProjectGridderBase.h file. However, this would induce the dependency on logging
// for everything which includes it. 
#include <gridding/AProjectGridderBase.tcc>

namespace askap {
  namespace synthesis {
    
    AWProjectVisGridder::AWProjectVisGridder(const boost::shared_ptr<IBasicIllumination const> &illum,
					     const double wmax, const int nwplanes,
					     const double cutoff, const int overSample,
					     const int maxSupport, const int limitSupport,
					     const int maxFeeds, const int maxFields, const double pointingTol,
					     const double paTol, const double freqTol,
					     const bool frequencyDependent, const std::string& name) :
      AProjectGridderBase(maxFeeds,maxFields, pointingTol, paTol, freqTol),	  			     
      WProjectVisGridder(wmax, nwplanes, cutoff, overSample, maxSupport, limitSupport, name),
      itsReferenceFrequency(0.0), itsIllumination(illum),
      itsFreqDep(frequencyDependent),
      itsMaxFeeds(maxFeeds), itsMaxFields(maxFields), itsSlopes(2, maxFeeds, maxFields,0.)
    {
      ASKAPDEBUGASSERT(itsIllumination);
      ASKAPCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
      ASKAPCHECK(maxFields>0, "Maximum number of fields must be one or more");
      ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
      ASKAPCHECK(maxSupport>0, "Maximum support must be greater than 0")
      itsName=name;
    }
    
    /// @brief copy constructor
    /// @details It is required to decouple internal array arrays, otherwise
    /// those arrays are shared between all cloned gridders of this type
    /// @note illumination model is copied as a pointer, so the same model is referenced
    /// @param[in] other input object
    AWProjectVisGridder::AWProjectVisGridder(const AWProjectVisGridder &other) :
      AProjectGridderBase(other), WProjectVisGridder(other), 
      itsReferenceFrequency(other.itsReferenceFrequency),
      itsIllumination(other.itsIllumination), itsFreqDep(other.itsFreqDep),
      itsMaxFeeds(other.itsMaxFeeds), itsMaxFields(other.itsMaxFields),
      itsSlopes(other.itsSlopes.copy()) {}
        
    /// Clone a copy of this Gridder
    IVisGridder::ShPtr AWProjectVisGridder::clone() {
      return IVisGridder::ShPtr(new AWProjectVisGridder(*this));
    }
    
    /// Initialize the indices into the cube.
    void AWProjectVisGridder::initIndices(const IConstDataAccessor& acc) {
      // calculate currentField
      indexField(acc);
      
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = acc.nRow();
      const int nChan = acc.nChannel();
      
      const int nPol = acc.nPol();
      itsCMap.resize(nSamples, nPol, nChan);
      itsCMap.set(0);
      
      const int cenw=(itsNWPlanes-1)/2;
      const casa::Vector<casa::RigidVector<double, 3> > &rotatedUVW = acc.rotatedUVW(getTangentPoint());
      
      
      for (int i=0; i<nSamples; ++i) {
           const int feed=acc.feed1()(i);
           ASKAPCHECK(feed<itsMaxFeeds, "Exceeded specified maximum number of feeds");
           ASKAPCHECK(feed>-1, "Illegal negative feed number");
	
           const double w=(rotatedUVW(i)(2))/(casa::C::c);
	
           for (int chan=0; chan<nChan; ++chan) {
                const double freq=acc.frequency()[chan];
                int iw=0;
                if (itsNWPlanes>1) {
                    iw=cenw+int(w*freq/itsWScale);
                }
                ASKAPCHECK(iw<itsNWPlanes,
                          "W scaling error: recommend allowing larger range of w, you have w="<<
                           w*freq<<" wavelengths");
                ASKAPCHECK(iw>-1,
                          "W scaling error: recommend allowing larger range of w, you have w="<<
                           w*freq<<" wavelengths, freq="<<freq/1e9<<" GHz, w(before rotation)="<<
                           acc.uvw()(i)(2)/casa::C::c*freq<<" wavelengths");
	  
                for (int pol=0; pol<nPol; ++pol) {
                     /// Order is (iw, chan, feed)
                     if (itsFreqDep) {
                         itsCMap(i, pol, chan)=iw+itsNWPlanes*(chan+nChan*(feed+itsMaxFeeds*currentField()));
                         ASKAPCHECK(itsCMap(i, pol, chan)<itsNWPlanes*itsMaxFeeds*itsMaxFields*nChan, 
                                    "CMap index too large");
                         ASKAPCHECK(itsCMap(i, pol, chan)>-1,"CMap index less than zero");
                     } else {
                         itsCMap(i, pol, chan)=iw+itsNWPlanes*(feed+itsMaxFeeds*currentField());
                         ASKAPCHECK(itsCMap(i, pol, chan)<itsNWPlanes*itsMaxFeeds*itsMaxFields, 
                                "CMap index too large");
                         ASKAPCHECK(itsCMap(i, pol, chan)>-1,"CMap index less than zero");
                     }
                }
	       }
      }
    }
    /// @brief initialise sum of weights
    /// @details We keep track the number of times each convolution function is used per
    /// channel and polarisation (sum of weights). This method is made virtual to be able
    /// to do gridder specific initialisation without overriding initialiseGrid.
    /// This method accepts no parameters as itsShape, itsNWPlanes, etc should have already
    /// been initialised by the time this method is called.
    void AWProjectVisGridder::initialiseSumOfWeights()
    {
      // this method is hopefully just a temporary stub until we figure out a better way of
      // managing a cache of convolution functions. It skips initialisation if itsSupport is
      // not zero, which means that some initialisation has been done before. 
      // Note, it is not a very good way of doing things!
      if (itsSupport == 0) {
          WProjectVisGridder::initialiseSumOfWeights();
      }
    }
    
/// @brief Initialise the gridding
/// @param axes axes specifications
/// @param shape Shape of output image: u,v,pol,chan
/// @param dopsf Make the psf?
void AWProjectVisGridder::initialiseGrid(const scimath::Axes& axes,  const casa::IPosition& shape, 
                                          const bool dopsf)
{
    WProjectVisGridder::initialiseGrid(axes,shape,dopsf);

    /// Limit the size of the convolution function since
    /// we don't need it finely sampled in image space. This
    /// will reduce the time taken to calculate it.
    const casa::uInt nx=std::min(maxSupport(), int(itsShape(0)));
    const casa::uInt ny=std::min(maxSupport(), int(itsShape(1)));

    ASKAPLOG_INFO_STR(logger, "Shape for calculating gridding convolution function = "
            << nx << " by " << ny << " pixels");

    // this is just a buffer in the uv-space, oversampling is
    // taken into account inside the UVPattern object (in the past we handled
    // oversampling explicitly by using qnx and qny instead of nx and ny and
    // passing 1 instead of itsOverSample, but it caused scaling problems for
    // offset feeds).
    initUVPattern(nx,ny, itsUVCellSize(0),itsUVCellSize(1),itsOverSample);
}

/// @brief Initialise the degridding
/// @param axes axes specifications
/// @param image Input image: cube: u,v,pol,chan
void AWProjectVisGridder::initialiseDegrid(const scimath::Axes& axes,
        const casa::Array<double>& image)
{
    WProjectVisGridder::initialiseDegrid(axes,image);      
    /// Limit the size of the convolution function since
    /// we don't need it finely sampled in image space. This
    /// will reduce the time taken to calculate it.
    const casa::uInt nx=std::min(maxSupport(), int(itsShape(0)));
    const casa::uInt ny=std::min(maxSupport(), int(itsShape(1)));

    ASKAPLOG_INFO_STR(logger, "Shape for calculating degridding convolution function = "
            << nx << " by " << ny << " pixels");

    // this is just a buffer in the uv-space, oversampling is
    // taken into account inside the UVPattern object (in the past we handled
    // oversampling explicitly by using qnx and qny instead of nx and ny and
    // passing 1 instead of itsOverSample, but it caused scaling problems for
    // offset feeds).
    initUVPattern(nx,ny, itsUVCellSize(0),itsUVCellSize(1),itsOverSample);
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

      validateCFCache(acc, hasSymmetricIllumination);
      
      
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nChan=itsFreqDep ? acc.nChannel() : 1;
      
      if(itsSupport==0) {
         itsConvFunc.resize(itsOverSample*itsOverSample*itsNWPlanes*itsMaxFeeds*itsMaxFields*nChan);
         itsSumWeights.resize(itsNWPlanes*itsMaxFeeds*itsMaxFields*nChan, itsShape(2), itsShape(3));
         itsSumWeights.set(0.0);
         if (isOffsetSupportAllowed()) {
             initConvFuncOffsets(itsNWPlanes*itsMaxFeeds*itsMaxFields);
	     }         
      }
      
      const int cenw=(itsNWPlanes-1)/2;
      
      /// Limit the size of the convolution function since
      /// we don't need it finely sampled in image space. This
      /// will reduce the time taken to calculate it.
      casa::uInt nx=std::min(maxSupport(), int(itsShape(0)));
      casa::uInt ny=std::min(maxSupport(), int(itsShape(1)));
      
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
      
      UVPattern &pattern = uvPattern();
      
      int nDone=0;
      for (int row=0; row<nSamples; ++row) {
           const int feed=acc.feed1()(row);
           if (!isCFValid(feed, currentField())) {
               makeCFValid(feed, currentField());
               nDone++;
               casa::MVDirection offset(acc.pointingDir1()(row).getAngle());
               itsSlopes(0, feed, currentField()) = isPSFGridder() ? 0. : sin(offset.getLong()
						   -out.getLong()) *cos(offset.getLat());
               itsSlopes(1, feed, currentField())= isPSFGridder() ? 0. : sin(offset.getLat())
                           *cos(out.getLat()) - cos(offset.getLat())*sin(out.getLat())
                           *cos(offset.getLong()-out.getLong());
	  
               const double parallacticAngle = hasSymmetricIllumination ? 0. : acc.feed1PA()(row);
	  
               for (int chan=0; chan<nChan; ++chan) {
	    
                    /// Extract illumination pattern for this channel
                    itsIllumination->getPattern(acc.frequency()[chan], pattern,
                          itsSlopes(0, feed, currentField()),
                          itsSlopes(1, feed, currentField()), parallacticAngle);
	    
                    scimath::fft2d(pattern.pattern(), false);
	    	    
	    
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
	      scimath::fft2d(thisPlane, true);
	      
	      // Now correct for normalization of FFT
	      thisPlane*=casa::Complex(1.0/(double(nx)*double(ny)));
	      maxCF/=double(nx)*double(ny);

	      const int zIndex=iw+itsNWPlanes*(chan+nChan*(feed+itsMaxFeeds*currentField()));
	      	      	    
	      // If the support is not yet set, find it and size the
	      // convolution function appropriately
	      
	      // by default the common support without offset is used
          CFSupport cfSupport(itsSupport);
          if (isSupportPlaneDependent() || (itsSupport == 0)) {
	          //  SynthesisParamsHelper::saveAsCasaImage("dbg.img", amplitude(thisPlane));
	          cfSupport = extractSupport(thisPlane);
	          const int support = cfSupport.itsSize;
	          
              ASKAPCHECK(support*itsOverSample<int(nx)/2,
                         "Overflowing convolution function - increase maxSupport or decrease overSample. "<<
                         "Current support size = "<<support<<" oversampling factor="<<itsOverSample<<
                         " image size nx="<<nx)
        
              cfSupport.itsSize = limitSupportIfNecessary(support); 	   
			  if (itsSupport == 0) {     
			      itsSupport = cfSupport.itsSize;
                  ASKAPLOG_INFO_STR(logger, "Number of planes in convolution function = "
				      << itsConvFunc.size()<<" or "<<itsConvFunc.size()/itsOverSample/itsOverSample<<
				        " before oversampling with factor "<<itsOverSample);
              }		      
              if (isOffsetSupportAllowed()) {
	              setConvFuncOffset(zIndex,cfSupport.itsOffsetU,cfSupport.itsOffsetV);
	          }
		      // just for log output					
		      const double cell=std::abs(itsUVCellSize(0)*(casa::C::c
							     /acc.frequency()[chan]));
		      ASKAPLOG_INFO_STR(logger, "CF cache w-plane="<<iw<<" feed="<<feed<<" field="<<currentField()<<
		             ": maximum extent = "<< support*cell << " (m) sampled at "<< cell/itsOverSample << " (m)"<<
		             " offset (m): "<<cfSupport.itsOffsetU*cell<<" "<<cfSupport.itsOffsetV*cell);
	      }
	      
	      // use either support determined for this particular plane or a generic one,
          // determined from the first plane (largest support as we have the largest w-term)
          const int support = isSupportPlaneDependent() ? cfSupport.itsSize : itsSupport;
	      
	      // Since we are decimating, we need to rescale by the
	      // decimation factor
	      const float rescale=float(itsOverSample*itsOverSample);
		  const int cSize=2*support+1;
	      for (int fracu=0; fracu<itsOverSample; fracu++) {
		for (int fracv=0; fracv<itsOverSample; fracv++) {
		  int plane=fracu+itsOverSample*(fracv+itsOverSample
						 *zIndex);
		  ASKAPDEBUGASSERT(plane>=0 && plane<int(itsConvFunc.size()));
		  itsConvFunc[plane].resize(cSize, cSize);
		  itsConvFunc[plane].set(0.0);
		  // Now cut out the inner part of the convolution function and
		  // insert it into the convolution function
		  for (int iy=-support; iy<support; iy++) {
		    for (int ix=-support; ix<support; ix++) {
		         ASKAPDEBUGASSERT((ix + support >= 0) && (iy + support >= 0));
                 ASKAPDEBUGASSERT(ix+support < int(itsConvFunc[plane].nrow()));
                 ASKAPDEBUGASSERT(iy+support < int(itsConvFunc[plane].ncolumn()));
                 ASKAPDEBUGASSERT((ix+cfSupport.itsOffsetU)*itsOverSample+fracu+nx/2 >= 0);
                 ASKAPDEBUGASSERT((iy+cfSupport.itsOffsetV)*itsOverSample+fracv+ny/2 >= 0);
                 ASKAPDEBUGASSERT((ix+cfSupport.itsOffsetU)*itsOverSample+fracu+nx/2 < int(thisPlane.nrow()));
                 ASKAPDEBUGASSERT((iy+cfSupport.itsOffsetV)*itsOverSample+fracv+ny/2 < int(thisPlane.ncolumn()));                      
		    
		      itsConvFunc[plane](ix+support, iy+support)
			        = rescale*thisPlane((ix+cfSupport.itsOffsetU)*itsOverSample+fracu+nx/2,
					    (iy+cfSupport.itsOffsetV)*itsOverSample+fracv+ny/2);
		    } // for ix
		  } // for iy
		   		  
		} // for fracv
	      } // for fracu
	    } // w loop
	  } // chan loop
	  
	} // row of the accessor
      }
      
      
      if (nDone == itsMaxFeeds*itsMaxFields*itsNWPlanes) {
            if (isSupportPlaneDependent()) {
                ASKAPLOG_INFO_STR(logger, "Convolution function cache has "<<itsConvFunc.size()<<" planes");
                ASKAPLOG_INFO_STR(logger, "Variable support size is used:");
                const size_t step = casa::max(itsConvFunc.size()/itsOverSample/itsOverSample/10,1);          
                for (size_t plane = 0; plane<itsConvFunc.size(); plane += step*itsOverSample*itsOverSample) {
                     ASKAPLOG_INFO_STR(logger, "CF cache plane "<<plane<<" ("<<plane/itsOverSample/itsOverSample<<
                        " prior to oversampling) shape is "<<itsConvFunc[plane].shape());
                }
            } else {
                ASKAPLOG_INFO_STR(logger, "Shape of convolution function = "
                     << itsConvFunc[0].shape() << " by "<< itsConvFunc.size() << " planes");
            }
      }
      
      ASKAPCHECK(itsSupport>0, "Support not calculated correctly");
      updateStats(nDone);
    }
    
    
    
    /// To finalize the transform of the weights, we use the following steps:
    /// 1. For each plane of the convolution function, transform to image plane
    /// and multiply by conjugate to get abs value squared.
    /// 2. Sum all planes weighted by the weight for that convolution function.
    void AWProjectVisGridder::finaliseWeights(casa::Array<double>& out) {
      
      ASKAPLOG_INFO_STR(logger, "Calculating sum of weights image");
      ASKAPDEBUGASSERT(itsShape.nelements()>=3);
      
      const int nx=itsShape(0);
      const int ny=itsShape(1);
      const int nPol=itsShape(2);
      const int nChan=itsShape(3);
      
      const int nZ=itsSumWeights.shape()(0);
      
      /// We must pad the convolution function to full size, reverse transform
      /// square, and sum multiplied by the corresponding weight
      const int cnx=std::min(maxSupport(), nx);
      const int cny=std::min(maxSupport(), ny);
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

	  // use either support determined for this particular plane or a generic one,
      // determined from the first plane (largest support as we have the largest w-term)
      const int support = (int(itsConvFunc[plane].nrow()) - 1) / 2;
      ASKAPDEBUGASSERT(itsConvFunc[plane].nrow() % 2 == 1);
      ASKAPDEBUGASSERT(itsConvFunc[plane].nrow() == itsConvFunc[plane].ncolumn());
      
      const std::pair<int,int> cfOffset = getConvFuncOffset(iz);

	  for (int iy=-support; iy<+support; ++iy) {
	    for (int ix=-support; ix<+support; ++ix) {
	      const int xPos = ix + ccenx + cfOffset.first;
	      const int yPos = iy + cceny + cfOffset.second;
	      if ((xPos<0) || (yPos<0) || (xPos>=int(thisPlane.nrow())) || (yPos>=int(thisPlane.ncolumn()))) {
	          continue;
	      }
	      thisPlane(xPos, yPos)=itsConvFunc[plane](ix+support, iy+support);
	    }
	  }	  
	  
	  float peak=real(casa::max(casa::abs(thisPlane)));
	  //	  ASKAPLOG_INFO_STR(logger, "Convolution function["<< iz << "] peak = "<< peak);
	  scimath::fft2d(thisPlane, false);
	  thisPlane*=casa::Complex(cnx*cny);
	  
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
      
      scimath::PaddingUtils::fftPad(cOut, out, paddingFactor());
 
      ASKAPLOG_INFO_STR(logger, 
			"Finished finalising the weights, the sum over all convolution functions is "<<totSumWt);	
    }


/// Correct for gridding convolution function
/// @param image image to be corrected
void AWProjectVisGridder::correctConvolution(casa::Array<double>& image)
{}
        
    int AWProjectVisGridder::cIndex(int row, int pol, int chan) {
      return itsCMap(row, pol, chan);
    }


/// @brief static method to create gridder
/// @details Each gridder should have a static factory method, which is
/// able to create a particular type of the gridder and initialise it with
/// the parameters taken form the given parset. It is assumed that the 
/// method receives a subset of parameters where the gridder name is already
/// taken out. 
/// @param[in] parset input parset file
/// @return a shared pointer to the gridder instance					 
IVisGridder::ShPtr AWProjectVisGridder::createGridder(const LOFAR::ParameterSet& parset)
{
  boost::shared_ptr<AWProjectVisGridder> gridder = createAProjectGridder<AWProjectVisGridder>(parset);
  const bool planeDependentSupport = parset.getBool("variablesupport",false);
  if (planeDependentSupport) {
      ASKAPLOG_INFO_STR(logger, "Support size will be calculated separately for each plane of the CF cache");
  } else {
      ASKAPLOG_INFO_STR(logger, "Common support size will be used for all planes of the CF cache");
  }
  gridder->planeDependentSupport(planeDependentSupport);
  
  const bool offsetSupport = parset.getBool("offsetsupport",false);
  ASKAPCHECK((!offsetSupport && !planeDependentSupport) || planeDependentSupport, 
             "offsetsupport option of the gridder should only be used together with variablesupport option");
  gridder->offsetSupport(offsetSupport);            
  return gridder;
}
    
    
  }
}
