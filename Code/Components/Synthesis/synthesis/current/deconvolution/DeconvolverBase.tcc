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
      ASKAPLOG_INFO_STR(decbaselogger, "Number of residual calculations = " << itsNumberResidualCalc);
    };
    
    template<class T, class FT>
    DeconvolverBase<T,FT>::DeconvolverBase(Vector<Array<T> >& dirty, Vector<Array<T> >& psf)
      : itsNumberResidualCalc(0)
    {
      init(dirty,psf);
    }

    template<class T, class FT>
    DeconvolverBase<T,FT>::DeconvolverBase(Array<T>& dirty, Array<T>& psf)
      : itsNumberResidualCalc(0)
    {
      itsNumberTerms=1;
      Vector<Array<T> > dirtyVec(1);
      dirtyVec(0)=dirty;
      Vector<Array<T> > psfVec(1);
      psfVec(0)=psf;
      init(dirtyVec, psfVec);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::init(Vector<Array<T> >& dirtyVec, Vector<Array<T> >& psfVec) {

      ASKAPCHECK(dirtyVec.shape().nelements()==psfVec.shape().nelements(), "Dirty images and PSF's not the same length");

      itsNumberTerms=dirtyVec.shape().nelements();

      itsDirty.resize(itsNumberTerms);
      itsPsf.resize(itsNumberTerms);
      itsXFR.resize(itsNumberTerms);
      itsModel.resize(itsNumberTerms);
      itsBackground.resize(itsNumberTerms);
      itsResidual.resize(itsNumberTerms);
      itsMask.resize(itsNumberTerms);
      itsWeight.resize(itsNumberTerms);
      itsWeightedMask.resize(itsNumberTerms);
      itsLipschitz.resize(itsNumberTerms);

      for (uInt term=0;term<itsNumberTerms;term++) {
	
	ASKAPASSERT(dirtyVec(term).shape().nelements());
	ASKAPASSERT(psfVec(term).shape().nelements());
	ASKAPASSERT(psfVec(term).shape().conform(dirtyVec(term).shape()));
	
	this->itsDirty(term)=dirtyVec(term);
	this->itsPsf(term)=psfVec(term);

	ASKAPLOG_INFO_STR(decbaselogger, "Dirty image[" << term << "] has shape: " << this->dirty(term).shape());

	this->model(term).resize(this->dirty(term).shape());
	this->model(term).set(T(0.0));
	this->background(term).resize(this->dirty(term).shape());
	this->background(term).set(T(0.0));
	this->residual(term).resize(this->dirty(term).shape());
	this->residual(term).set(T(0.0));
	
	this->XFR(term).resize(this->psf(term).shape());
	this->XFR(term).set(FT(0.0));

	casa::setReal(this->XFR(term), this->psf(term));
	scimath::fft2d(this->XFR(term), true);
	itsLipschitz(term)=casa::max(casa::real(casa::abs(this->XFR(term))));
	ASKAPLOG_INFO_STR(decbaselogger, "For term " << term << ", Lipschitz number = " << itsLipschitz(term));
      }

      itsDS = boost::shared_ptr<DeconvolverState<T> >(new DeconvolverState<T>());
      ASKAPASSERT(itsDS);
      itsDC = boost::shared_ptr<DeconvolverControl<T> >(new DeconvolverControl<T>());
      ASKAPASSERT(itsDC);
      itsDM = boost::shared_ptr<DeconvolverMonitor<T> >(new DeconvolverMonitor<T>());
      ASKAPASSERT(itsDM);

    }      
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::configure(const LOFAR::ParameterSet& parset)
    {        
      this->itsDC->configure(parset);
      this->itsDM->configure(parset);
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::setModel(const Array<T> model, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      this->itsModel(term)=model.copy();
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::model(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsModel(term);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setResidual(const Array<T> residual, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      this->itsResidual(term)=residual.copy();
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::residual(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsResidual(term);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setBackground(const Array<T> background, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      this->itsBackground(term)=background.copy();
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::background(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsBackground(term);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateDirty(Array<T> dirty, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      if (!dirty.shape().conform(this->dirty(term).shape())) {
        throw(AskapError("Updated dirty image has different shape"));
      }
      this->itsDirty(term)=dirty;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::deconvolve()
    {
      throw(AskapError("Called base class deconvolver"));
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setDirty(Array<T> Dirty, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      this->itsDirty(term)=Dirty;
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::dirty(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsDirty(term);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setPsf(Array<T> Psf, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      this->itsPsf(term)=Psf;
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::psf(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsPsf(term);
    }
    
    template<class T, class FT>
    Array<FT> & DeconvolverBase<T,FT>::XFR(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsXFR(term);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setMask(Array<T> mask, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      this->itsMask(term)=mask;
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::mask(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsMask(term);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setWeight(Array<T> weight, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      this->itsMask(term)=weight;
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::weight(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsWeight(term);
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
      ASKAPLOG_INFO_STR(logger, "Initialising mask and weight images"); 

      // Always check shapes on initialise
      this->validateShapes();

      for (uInt term=0;term<itsNumberTerms;term++) {
	this->residual(term).resize(this->dirty(term).shape());
	this->residual(term)=this->dirty(term).copy();

	// First deal with the mask
	if(this->mask().shape().nonDegenerate().conform(this->dirty().nonDegenerate().shape())) { // mask exists
	  if(this->weight().shape().nonDegenerate().conform(this->dirty().nonDegenerate().shape())) {
	    ASKAPLOG_INFO_STR(logger, "Setting weighted mask image");
	    itsWeightedMask(term)=this->mask(term)*this->weight(term);
	    ASKAPASSERT(itsWeightedMask(term).shape().nonDegenerate().conform(this->dirty(term).shape().nonDegenerate()));
	  }
	  else { // only mask exists
	    ASKAPLOG_INFO_STR(logger, "Setting mask image"); 
	    itsWeightedMask(term)=this->mask(term);
	    ASKAPASSERT(itsWeightedMask(term).shape().nonDegenerate().conform(this->dirty(term).shape().nonDegenerate()));
	  }
	} 
	else { // no mask
	  if(this->weight(term).shape().nonDegenerate().conform(this->dirty(term).nonDegenerate().shape())) {
	    ASKAPLOG_INFO_STR(logger, "Setting weights image");
	    itsWeightedMask(term)=this->weight(term);
	    ASKAPASSERT(itsWeightedMask(term).shape().nonDegenerate().conform(this->dirty(term).shape().nonDegenerate()));
	  }
	  else { // we got nuthin'
	    ASKAPLOG_INFO_STR(logger, "No weights or mask image for term " << term);
	  }
	}
      }

      // Now we need to find the peak and support of the PSF
      itsPeakPSFVal.resize(itsNumberTerms);
      itsPeakPSFPos.resize(itsNumberTerms);

      for (uInt term=0;term<itsNumberTerms;term++) {
	casa::IPosition minPos;
	casa::IPosition maxPos;
	T minVal, maxVal;
	ASKAPLOG_INFO_STR(logger, "Validating PSF for term " << term);
	casa::minMax(minVal, maxVal, minPos, maxPos, this->psf(term));
	ASKAPLOG_INFO_STR(logger, "Maximum of PSF = " << maxVal << " at " << maxPos);
	ASKAPLOG_INFO_STR(logger, "Minimum of PSF = " << minVal << " at " << minPos);
	itsPeakPSFVal(term) = maxVal;
	itsPeakPSFPos(term) = maxPos;
	if (term==0) {
	  // Check the peak of the PSF - it should be at nx/2,ny/2
	  if(!(maxPos==this->psf(term).shape()/2)) {
	    throw(AskapError("Peak of PSF is not at center"));
	  };
	}
      }
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::finalise() {
      updateResiduals(itsModel);
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateResiduals(Vector<Array<T> >& model)
    {
      ASKAPCHECK(model.shape()==itsNumberTerms, "Number of terms in model " << model.shape()
		 << " not same as number of terms specified "
		 << itsNumberTerms);

      for (uInt term=0;term<itsNumberTerms;term++) {
	Array<FT> work;
	// Find residuals for current model model
	work.resize(model(term).shape());
	work.set(FT(0.0));
	casa::setReal(work, model(term)-this->itsBackground(term));
	scimath::fft2d(work, true);
	work=this->XFR(term)*work;
	scimath::fft2d(work, false);
	this->residual(term)=this->dirty(term)-real(work);
      
	itsNumberResidualCalc+=1;
      }
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateResiduals(Array<T> & model)
    {
      Vector<Array<T> > modelVec(1);
      modelVec(0)=model;
      updateResiduals(modelVec);
    }

  } // namespace synthesis
  
} // namespace askap


