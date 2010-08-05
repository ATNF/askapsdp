/// @file
/// @brief Class for a deconvolver based on the Hogbom CLEAN
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

#include <askap/AskapLogging.h>
ASKAP_LOGGER(dechogbomlogger, ".deconvolution.hogbom");

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverHogbom.h>

namespace askap {

  namespace synthesis {

    /// @brief Class for a deconvolver based on the Hogbom Clean
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverHogbom<Double, DComplex>
    /// @ingroup Deconvolver

    template<class T, class FT>
    DeconvolverHogbom<T,FT>::~DeconvolverHogbom() {
    };

    template<class T, class FT>
    DeconvolverHogbom<T,FT>::DeconvolverHogbom(Array<T>& dirty, Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf)
    {
      this->model().resize(this->dirty().shape());
      this->model().set(T(0.0));
    };

    template<class T, class FT>
    void DeconvolverHogbom<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();
    }

    template<class T, class FT>
    bool DeconvolverHogbom<T,FT>::deconvolve()
    {

      this->initialise();

      ASKAPLOG_INFO_STR(dechogbomlogger, "Performing Hogbom CLEAN for " << this->control()->targetIter() << " iterations");
      do {
        this->oneIteration();
        this->monitor()->monitor(*(this->state()));
        this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));

      ASKAPLOG_INFO_STR(dechogbomlogger, "Performed Hogbom CLEAN for " << this->state()->currentIter() << " iterations");

      ASKAPLOG_INFO_STR(dechogbomlogger, this->control()->terminationString());

      this->finalise();

      return True;
    }
    
    template<class T, class FT>
    void DeconvolverHogbom<T,FT>::configure(const LOFAR::ParameterSet& parset)
    {        
      DeconvolverBase<T,FT>::configure(parset);
      this->control()->setGain(parset.getFloat("gain", 0.1));
      this->control()->setPSFWidth(parset.getInt("psfwidth", 0));
    }

    // This contains the heart of the Hogbom Clean algorithm
    template<class T, class FT>
    bool DeconvolverHogbom<T,FT>::oneIteration()
    {
      bool isMasked(this->itsWeightedMask.shape().conform(this->residual().shape()));

      // Find peak in residual image
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      if (isMasked) {
        casa::minMaxMasked(minVal, maxVal, minPos, maxPos, this->residual(),
                           this->itsWeightedMask);
      }
      else {
        casa::minMax(minVal, maxVal, minPos, maxPos, this->residual());
      }
      //
      ASKAPLOG_INFO_STR(dechogbomlogger, "Maximum = " << maxVal << " at location " << maxPos);
      ASKAPLOG_INFO_STR(dechogbomlogger, "Minimum = " << minVal << " at location " << minPos);
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

      casa::IPosition residualShape(this->residual().shape());
      casa::IPosition psfShape(this->psf().shape());
      casa::uInt ndim(this->residual().shape().size());

      casa::IPosition residualStart(ndim,0), residualEnd(ndim,0), residualStride(ndim,1);
      casa::IPosition psfStart(ndim,0), psfEnd(ndim,0), psfStride(ndim,1);

      Int psfWidth=this->psf().shape()(0);
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
        // residual image and psf can be different sizes
        // Next two lines are ALWAYS correct
        residualStart(dim)=max(0, Int(absPeakPos(dim)-psfWidth));
        residualEnd(dim)=min(Int(absPeakPos(dim)+psfWidth-1), Int(residualShape(dim)-1));
        // Now we have to deal with the PSF. Here we want to use enough of the
        // PSF to clean the residual image.
        psfStart(dim)=max(0, Int(this->itsPeakPSFPos(dim)-(absPeakPos(dim)-residualStart(dim))));
        psfEnd(dim)=min(Int(this->itsPeakPSFPos(dim)-(absPeakPos(dim)-residualEnd(dim))),
                        Int(psfShape(dim)-1));
      }
      
      casa::Slicer residualSlicer(residualStart, residualEnd, residualStride, Slicer::endIsLast);
      casa::Slicer psfSlicer(psfStart, psfEnd, psfStride, Slicer::endIsLast);
      if(!(residualSlicer.length()==psfSlicer.length())||!(residualSlicer.stride()==psfSlicer.stride())) {
	ASKAPLOG_INFO_STR(dechogbomlogger, "Peak of PSF  : " << this->itsPeakPSFPos );
	ASKAPLOG_INFO_STR(dechogbomlogger, "Peak of residual: " << absPeakPos );
	ASKAPLOG_INFO_STR(dechogbomlogger, "PSF width    : " << psfWidth );
	ASKAPLOG_INFO_STR(dechogbomlogger, "Residual start  : " << residualStart << " end: " << residualEnd );
	ASKAPLOG_INFO_STR(dechogbomlogger, "PSF   start  : " << psfStart << " end: " << psfEnd );
	ASKAPLOG_INFO_STR(dechogbomlogger, "Residual slicer : " << residualSlicer );
	ASKAPLOG_INFO_STR(dechogbomlogger, "PSF slicer   : " << psfSlicer );

        throw AskapError("Mismatch in slicers for residual and psf images");
      }
      
      // Add to model
      this->model()(absPeakPos) = this->model()(absPeakPos) + this->control()->gain()*absPeakVal;      
      
      // Subtract entire PSF from residual image
      this->residual()(residualSlicer) = this->residual()(residualSlicer)
        - this->control()->gain()*absPeakVal*this->psf()(psfSlicer);
      
      return True;
    }

  } // namespace synthesis

} // namespace askap


