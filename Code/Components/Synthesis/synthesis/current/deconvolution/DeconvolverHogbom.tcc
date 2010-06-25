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
      this->model() = this->dirty().copy();
      this->model().set(T(0.0));
    };

    template<class T, class FT>
    void DeconvolverHogbom<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();

      ASKAPASSERT(this->mask().shape()==this->weight().shape());

      ASKAPLOG_INFO_STR(logger, "Calculating weighted mask");
      itsWeightedMask=this->mask()/sqrt(this->weight());

      ASKAPASSERT(itsWeightedMask.shape()==this->dirty().shape());
    }

    template<class T, class FT>
    bool DeconvolverHogbom<T,FT>::deconvolve()
    {

      this->initialise();

      ASKAPLOG_INFO_STR(logger, "Performing Hogbom CLEAN for " << this->control()->targetIter() << "iterations");
      do {
        this->oneIteration();
        this->monitor()->monitor(*(this->state()));
        this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));

      ASKAPLOG_INFO_STR(logger, "Performed Hogbom CLEAN for " << this->state()->currentIter() << "iterations");

      ASKAPLOG_INFO_STR(logger, this->control()->terminationString());

      this->finalise();

      return True;
    }
    
    // This contains the heart of the Hogbom Clean algorithm
    template<class T, class FT>
    bool DeconvolverHogbom<T,FT>::oneIteration()
    {
      bool isMasked(itsWeightedMask.shape()==this->dirty().shape());

      // Find peak in dirty image
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      if (isMasked) {
        casa::minMaxMasked(minVal, maxVal, minPos, maxPos, this->dirty(),
                           itsWeightedMask);
      }
      else {
        casa::minMax(minVal, maxVal, minPos, maxPos, this->dirty());
      }
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
      if(isMasked) {
        absPeakVal=absPeakVal*sqrt(this->weight()(absPeakPos));
      }

      this->state()->setPeakResidual(absPeakVal);
      this->state()->setObjectiveFunction(absPeakVal);

      // Has this terminated for any reason?
      if(this->control()->terminate(*(this->state()))) {
        return True;
      }

      // Add to model
      this->model()(absPeakPos) = this->model()(absPeakPos)
        + this->control()->gain()*maxVal;

      // Subtract from dirty image
      this->dirty()(absPeakPos) = this->dirty()(absPeakPos)
        - this->control()->gain()*maxVal;

      return True;
    }

  } // namespace synthesis

} // namespace askap


