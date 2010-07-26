/// @file
/// @brief Class for a deconvolver based on CLEANing with basis functions.
/// @details This concrete class defines a deconvolver used to estimate an
/// image from a dirty image, psf optionally using a mask and a weights image.
/// @ingroup Deconvolver
///  
///
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverBasisFunction.h>

namespace askap {

  namespace synthesis {

    /// @brief Class for a deconvolver based on the BasisFunction Clean
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverBasisFunction<Double, DComplex>
    /// @ingroup Deconvolver

    template<class T, class FT>
    DeconvolverBasisFunction<T,FT>::~DeconvolverBasisFunction() {
    };

    template<class T, class FT>
    void DeconvolverBasisFunction<T,FT>::setBasisFunction(boost::shared_ptr<BasisFunction<T> > bf) {
      itsBasisFunction=bf;
    };

    template<class T, class FT>
    boost::shared_ptr<BasisFunction<T> > DeconvolverBasisFunction<T,FT>::basisFunction() {
      return itsBasisFunction;
    };

    template<class T, class FT>
    DeconvolverBasisFunction<T,FT>::DeconvolverBasisFunction(Array<T>& dirty, Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf), itsPeakPSFPos(IPosition(0)),
        itsPeakPSFVal(T(0.0))
    {
      this->model() = this->dirty().copy();
      this->model().set(T(0.0));
    };

    template<class T, class FT>
    void DeconvolverBasisFunction<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();

      // First deal with the mask
      ASKAPASSERT(this->mask().shape()==this->weight().shape());

      ASKAPLOG_INFO_STR(logger, "Calculating weighted mask");
      itsWeightedMask=this->mask()*sqrt(this->weight()/max(this->weight()));

      ASKAPASSERT(itsWeightedMask.shape()==this->dirty().shape());

      // Now we need to find the peak and support of the PSF
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      casa::minMax(minVal, maxVal, minPos, maxPos, this->psf());
      ASKAPLOG_INFO_STR(logger, "Maximum of PSF = " << maxVal << " at " << maxPos);
      ASKAPLOG_INFO_STR(logger, "Minimum of PSF = " << minVal << " at " << minPos);
      itsPeakPSFVal = maxVal;
      itsPeakPSFPos = maxPos;

      // Check the peak of the PSF - it should be at nx/2,ny/2
      if(!(maxPos==this->psf().shape()/2)) {
	throw(AskapError("Peak of PSF is not at center"));
      };
      // Now calculate the convolutions of the dirty images and PSFs 
      // with the basis functions
      IPosition stackShape(3, this->psf().shape()[0], this->psf().shape()[1],
                           this->itsBasisFunction->numberTerms());

      itsDirtyBasisFunction.resize(stackShape);
      itsPSFBasisFunction.resize(stackShape);
      for (uInt term=0;term<this->itsBasisFunction->numberTerms();term++) {
        Cube<T>(itsDirtyBasisFunction).xyPlane(term)=this->dirty();
        Cube<T>(itsPSFBasisFunction).xyPlane(term)=this->psf();
      }

    }

    template<class T, class FT>
    bool DeconvolverBasisFunction<T,FT>::deconvolve()
    {

      this->initialise();

      ASKAPLOG_INFO_STR(logger, "Performing BasisFunction CLEAN for "
                        << this->control()->targetIter() << " iterations");
      do {
        this->oneIteration();
        this->monitor()->monitor(*(this->state()));
        this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));

      ASKAPLOG_INFO_STR(logger, "Performed BasisFunction CLEAN for "
                        << this->state()->currentIter() << " iterations");

      ASKAPLOG_INFO_STR(logger, this->control()->terminationString());

      this->finalise();

      return True;
    }

    template<class T, class FT>
    bool DeconvolverBasisFunction<T,FT>::oneIteration() {
      if(itsBasisFunction->isOrthogonal()) {
        return oneIterationOrthogonal();
      }
      else {
        return oneIterationNonOrthogonal();
      }
    }

    template<class T, class FT>
    bool DeconvolverBasisFunction<T,FT>::oneIterationNonOrthogonal() {
      throw(AskapError("Non-orthogonal basis functions not yet implemented"));
    }

    
    // This contains the heart of the BasisFunction Clean algorithm
    // The dirty image and psfs are intrinsically two dimensional
    // but are expanded by projection onto the basis functions
    // This version is for orthogonal basis functions.
    template<class T, class FT>
    bool DeconvolverBasisFunction<T,FT>::oneIterationOrthogonal()
    {
      bool isMasked(itsWeightedMask.shape()==this->itsDirtyBasisFunction.shape());

      // Find peak in dirty image cube
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      if (isMasked) {
        casa::minMaxMasked(minVal, maxVal, minPos, maxPos, this->itsDirtyBasisFunction,
                           itsWeightedMask);
      }
      else {
        casa::minMax(minVal, maxVal, minPos, maxPos, this->itsDirtyBasisFunction);
      }
      //
      ASKAPLOG_INFO_STR(logger, "Maximum = " << maxVal << " at location " << maxPos);
      ASKAPLOG_INFO_STR(logger, "Minimum = " << minVal << " at location " << minPos);
      T absPeakVal;
      casa::IPosition absPeakPos;
      if(abs(minVal)<abs(maxVal)) {
        absPeakVal=maxVal;
        absPeakPos=maxPos;
      }
      else {
        absPeakVal=minVal;
        absPeakPos=minPos;
      }

      this->state()->setPeakResidual(absPeakVal);
      this->state()->setObjectiveFunction(absPeakVal);
      this->state()->setTotalFlux(sum(this->model()));

      // Has this terminated for any reason?
      if(this->control()->terminate(*(this->state()))) {
        return True;
      }

      casa::IPosition dirtyShape(this->itsDirtyBasisFunction.shape());
      casa::IPosition psfShape(this->itsPSFBasisFunction.shape());
      casa::uInt ndim(this->itsDirtyBasisFunction.shape().size());

      casa::IPosition dirtyStart(ndim,0), dirtyEnd(ndim,0), dirtyStride(ndim,1);
      casa::IPosition psfStart(ndim,0), psfEnd(ndim,0), psfStride(ndim,1);

      Int psfWidth=this->itsPSFBasisFunction.shape()(0);
      psfWidth=(psfWidth-psfWidth%2)/2;

      // Only use the specified psfWidth if it makes sense
      if((this->control()->psfWidth()>0)&&(this->control()->psfWidth()<psfWidth)) {
        psfWidth=(this->control()->psfWidth()-this->control()->psfWidth()%2);
      }
      for (uInt dim=0;dim<ndim;dim++) {
        // Wrangle the start, end, and shape into consistent form. It took me 
        // quite a while to figure this out (slow brain day) so it may be
        // that there are some edge cases for which it fails.
        // Note that the psfWidth can be less than the maximum, and that the
        // dirty image and psf can be different sizes
        // Next two lines are ALWAYS correct
        dirtyStart(dim)=max(0, Int(absPeakPos(dim)-psfWidth));
        dirtyEnd(dim)=min(Int(absPeakPos(dim)+psfWidth-1), Int(dirtyShape(dim)-1));
        // Now we have to deal with the PSF. Here we want to use enough of the
        // PSF to clean the dirty image.
        psfStart(dim)=max(0, Int(itsPeakPSFPos(dim)-(absPeakPos(dim)-dirtyStart(dim))));
        psfEnd(dim)=min(Int(itsPeakPSFPos(dim)-(absPeakPos(dim)-dirtyEnd(dim))),
                        Int(psfShape(dim)-1));
      }

      // For the model, we just have to update using the optimum plane
      uInt optimumPlane(absPeakPos(2));
      psfStart(2)=psfEnd(2)=optimumPlane;
      dirtyStart(2)=dirtyEnd(2)=optimumPlane;

      casa::Slicer dirtySlicer(dirtyStart, dirtyEnd, dirtyStride, Slicer::endIsLast);
      casa::Slicer psfSlicer(psfStart, psfEnd, psfStride, Slicer::endIsLast);
      if(!(dirtySlicer.length()==psfSlicer.length())||!(dirtySlicer.stride()==psfSlicer.stride())) {
        ASKAPLOG_INFO_STR(logger, "Peak of PSF  : " << itsPeakPSFPos );
        ASKAPLOG_INFO_STR(logger, "Peak of dirty: " << absPeakPos );
        ASKAPLOG_INFO_STR(logger, "PSF width    : " << psfWidth );
        ASKAPLOG_INFO_STR(logger, "Dirty start  : " << dirtyStart << " end: " << dirtyEnd );
        ASKAPLOG_INFO_STR(logger, "PSF   start  : " << psfStart << " end: " << psfEnd );
        ASKAPLOG_INFO_STR(logger, "Dirty slicer : " << dirtySlicer );
        ASKAPLOG_INFO_STR(logger, "PSF slicer   : " << psfSlicer );
        throw AskapError("Mismatch in slicers for dirty and psf images");
      }
        
      // Add to model
      this->model()(dirtySlicer) = this->model()(dirtySlicer)
        + this->control()->gain()*absPeakVal*itsBasisFunction->basisFunction()(dirtySlicer);
      
      // Subtract PSF for this plane from dirty image for the same plane
      // Since the basis functions are orthogonal, we make the 
      // approximation that the psfs and dirty images
      // are also orthogonal
      this->itsDirtyBasisFunction(dirtySlicer) = this->itsDirtyBasisFunction(dirtySlicer)
        - this->control()->gain()*absPeakVal*this->itsPSFBasisFunction(psfSlicer);
        
      return True;
    }

  } // namespace synthesis

} // namespace askap


