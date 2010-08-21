/// @file
/// @brief Base class for a deconvolver
/// @details This interface class defines a deconvolver used to estimate an
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
ASKAP_LOGGER(decbaselogger, ".deconvolution.base");

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <fft/FFTWrapper.h>

#include <string>

#include <deconvolution/DeconvolverBase.h>
#include <deconvolution/DeconvolverState.h>

namespace askap {
  
  namespace synthesis {
    
    template<class T, class FT>
    DeconvolverBase<T,FT>::~DeconvolverBase() {
    };
    
    template<class T, class FT>
    DeconvolverBase<T,FT>::DeconvolverBase(Array<T> dirty, Array<T> psf)
      : itsDirty(dirty), itsPSF(psf)
    {
      ASKAPASSERT(itsDirty.shape().size());
      ASKAPASSERT(itsPSF.shape().size());
      ASKAPASSERT(itsPSF.shape().conform(itsDirty.shape()));
      itsDS = boost::shared_ptr<DeconvolverState<T> >(new DeconvolverState<T>());
      ASKAPASSERT(itsDS);
      itsDC = boost::shared_ptr<DeconvolverControl<T> >(new DeconvolverControl<T>());
      ASKAPASSERT(itsDC);
      itsDM = boost::shared_ptr<DeconvolverMonitor<T> >(new DeconvolverMonitor<T>());
      ASKAPASSERT(itsDM);

      this->model().resize(this->dirty().shape());
      this->model().set(T(0.0));
      this->background().resize(this->dirty().shape());
      this->background().set(T(0.0));
      this->residual().resize(this->dirty().shape());
      this->residual().set(T(0.0));

      itsXFR.resize(this->psf().shape());
      itsXFR.set(FT(0.0));
      casa::setReal(itsXFR, this->psf());
      scimath::fft2d(itsXFR, true);
      itsLipschitz=casa::max(casa::real(casa::abs(this->itsXFR)));
      ASKAPLOG_INFO_STR(decbaselogger, "Lipschitz number = " << itsLipschitz);
    };
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::configure(const LOFAR::ParameterSet& parset)
    {        
      this->itsDC->configure(parset);
      this->itsDM->configure(parset);
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::setModel(const Array<T> model) {
      itsModel = model.copy();
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setBackground(const Array<T> background) {
      itsBackground = background.copy();
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateDirty(Array<T> dirty) {
      if (!dirty.shape().conform(itsDirty.shape())) {
        throw(AskapError("Updated dirty image has different shape"));
      }
      itsDirty = dirty;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::deconvolve()
    {
      throw(AskapError("Called base class deconvolver"));
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setMask(Array<T>  mask) {
      itsMask = mask;
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setWeight(Array<T>  weight) {
      itsWeight = weight;
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::mask()
    {
      return itsMask;
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::weight()
    {
      return itsWeight;
    }
    
    template<class T, class FT>
    boost::shared_ptr<DeconvolverControl<T> > DeconvolverBase<T,FT>::control() const
    {
      ASKAPASSERT(itsDC);
      return itsDC;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::setControl(boost::shared_ptr<DeconvolverControl<T> > DC)
    {
      itsDC = DC;
      ASKAPASSERT(itsDC);
      return True;
    }
    
    template<class T, class FT>
    boost::shared_ptr<DeconvolverMonitor<T> > DeconvolverBase<T,FT>::monitor() const
    {
      ASKAPASSERT(itsDM);
      return itsDM;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::setMonitor(boost::shared_ptr<DeconvolverMonitor<T> > DM)
    {
      itsDM = DM;
      ASKAPASSERT(itsDM);
      return True;
    }
    
    template<class T, class FT>
    boost::shared_ptr<DeconvolverState<T> > DeconvolverBase<T,FT>::state() const
    {
      ASKAPASSERT(itsDS);
      return itsDS;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::setState(boost::shared_ptr<DeconvolverState<T> > DS)
    {
      itsDS = DS;
      ASKAPASSERT(itsDS);
      return True;
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::validateShapes()
    {
      ASKAPASSERT(this->model().shape().size());
      // The model and dirty image shapes only need to agree on the
      // first two axes
      ASKAPASSERT(this->model().shape()[0]==this->dirty().shape()[0]);
      ASKAPASSERT(this->model().shape()[1]==this->dirty().shape()[1]);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::initialise()
    {
      // Always check shapes on initialise
      this->validateShapes();

      // Initialise the residual image
      this->residual().resize(this->dirty().shape());
      this->residual()=this->dirty().copy();

      // First deal with the mask
      if(this->mask().shape().conform(this->dirty().shape())) { // mask exists
	if(this->weight().shape().conform(this->dirty().shape())) {
	  ASKAPLOG_INFO_STR(logger, "Calculating weighted mask image");
	  itsWeightedMask=this->mask()*sqrt(this->weight()/max(this->weight()));
	  ASKAPASSERT(itsWeightedMask.shape().conform(this->dirty().shape()));
	}
	else { // only mask exists
	  ASKAPLOG_INFO_STR(logger, "Setting mask"); 
	  itsWeightedMask=this->mask();
	  ASKAPASSERT(itsWeightedMask.shape().conform(this->dirty().shape()));
	}
      } 
      else { // no mask
	if(this->weight().shape().conform(this->dirty().shape())) { // weights only
	  ASKAPLOG_INFO_STR(logger, "Calculating normalised weights image");
	  itsWeightedMask=sqrt(this->weight()/max(this->weight()));
	  ASKAPASSERT(itsWeightedMask.shape().conform(this->dirty().shape()));
	}
	else { // we got nuthin'
	  ASKAPLOG_INFO_STR(logger, "No weights or mask image");
	}
      }

      // Now we need to find the peak and support of the PSF
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      ASKAPLOG_INFO_STR(logger, "Validating PSF");
      casa::minMax(minVal, maxVal, minPos, maxPos, this->psf());
      ASKAPLOG_INFO_STR(logger, "Maximum of PSF = " << maxVal << " at " << maxPos);
      ASKAPLOG_INFO_STR(logger, "Minimum of PSF = " << minVal << " at " << minPos);
      itsPeakPSFVal = maxVal;
      itsPeakPSFPos = maxPos;

      // Check the peak of the PSF - it should be at nx/2,ny/2
      if(!(maxPos==this->psf().shape()/2)) {
	throw(AskapError("Peak of PSF is not at center"));
      };

    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::finalise() {
      updateResiduals(this->model());
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateResiduals(Array<T>& model)
    {
      Array<FT> work;
      // Find residuals for current model model
      work.resize(model.shape());
      work.set(FT(0.0));
      casa::setReal(work, model);
      scimath::fft2d(work, true);
      work=this->xfr()*work;
      scimath::fft2d(work, false);
      this->residual()=this->dirty()-real(work);
    }

  } // namespace synthesis
  
} // namespace askap


