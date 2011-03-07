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
      auditAllMemory();
    };
    
    template<class T, class FT>
    DeconvolverBase<T,FT>::DeconvolverBase(Vector<Array<T> >& dirty, Vector<Array<T> >& psf)
    {
      init(dirty,psf);
    }

    template<class T, class FT>
    DeconvolverBase<T,FT>::DeconvolverBase(Array<T>& dirty, Array<T>& psf)
    {
      Vector<Array<T> > dirtyVec(1);
      dirtyVec(0)=dirty.nonDegenerate();
      Vector<Array<T> > psfVec(1);
      psfVec(0)=psf.nonDegenerate();
      init(dirtyVec, psfVec);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::init(Vector<Array<T> >& dirtyVec, Vector<Array<T> >& psfVec) {

      ASKAPCHECK(psfVec.nelements()==dirtyVec.nelements(),
		 "Vectors of dirty images and PSF's not same length");

      itsNumberTerms=dirtyVec.nelements();

      itsDirty.resize(itsNumberTerms);
      itsPsf.resize(itsNumberTerms);
      itsModel.resize(itsNumberTerms);
      itsWeight.resize(itsNumberTerms);
      itsLipschitz.resize(itsNumberTerms);

      ASKAPLOG_INFO_STR(decbaselogger, "There are " << itsNumberTerms << " dirty images");

      for (uInt term=0;term<itsNumberTerms;term++) {
	
	ASKAPASSERT(dirtyVec(term).nonDegenerate().shape().nelements()==2);
	ASKAPASSERT(psfVec(term).nonDegenerate().shape().nelements()==2);
	
	this->itsDirty(term)=dirtyVec(term).nonDegenerate();
	this->itsPsf(term)=psfVec(term).nonDegenerate();

	ASKAPASSERT(this->itsPsf(term).shape().conform(this->itsDirty(term).shape()));

	ASKAPLOG_INFO_STR(decbaselogger, "Dirty image(" << term << ") has shape: "
			  << this->dirty(term).shape());

	this->model(term).resize(this->dirty(term).shape());
	this->model(term).set(T(0.0));
      }

      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      ASKAPLOG_INFO_STR(decbaselogger, "Validating PSF");
      casa::minMax(minVal, maxVal, minPos, maxPos, this->psf(0));
      
      uInt nx(this->psf(0).shape()(0));
      uInt ny(this->psf(0).shape()(1));
      
      ASKAPCHECK((uInt(maxPos(0))!=(nx/2-1))||(uInt(maxPos(1))!=(ny/2-1)), "Peak of PSF(0) is not at centre pixels");
      
      ASKAPLOG_INFO_STR(decbaselogger, "Maximum of PSF(0) = " << maxVal << " at " << maxPos);
      ASKAPLOG_INFO_STR(decbaselogger, "Minimum of PSF(0) = " << minVal << " at " << minPos);
      this->itsPeakPSFVal = maxVal;
      this->itsPeakPSFPos(0)=maxPos(0);
      this->itsPeakPSFPos(1)=maxPos(1);
      
      itsDS = boost::shared_ptr<DeconvolverState<T> >(new DeconvolverState<T>());
      ASKAPASSERT(itsDS);
      itsDC = boost::shared_ptr<DeconvolverControl<T> >(new DeconvolverControl<T>());
      ASKAPASSERT(itsDC);
      itsDM = boost::shared_ptr<DeconvolverMonitor<T> >(new DeconvolverMonitor<T>());
      ASKAPASSERT(itsDM);

      auditAllMemory();

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
      this->itsModel(term)=model.nonDegenerate().copy();
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::model(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsModel(term);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateDirty(Array<T>& dirty) {
      if (!dirty.shape().nonDegenerate().conform(this->dirty(0).shape())) {
        throw(AskapError("Updated dirty image has different shape"));
      }
      this->itsDirty.resize(1);
      this->itsDirty(0)=dirty.nonDegenerate();
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateDirty(Vector<Array<T> >& dirtyVec) {
      if (dirtyVec.nelements()!=this->itsDirty.nelements()) {
        throw(AskapError("Updated dirty image has different shape"));
      }
      this->itsDirty.resize(dirtyVec.nelements());
      for (uInt term=0;term<dirtyVec.nelements();term++) {
	if (!dirtyVec(term).nonDegenerate().shape().conform(this->itsDirty(term).nonDegenerate().shape())) {
	  throw(AskapError("Updated dirty image has different shape from original"));
	}
	this->itsDirty(term)=dirtyVec(term).nonDegenerate();
      }
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::deconvolve()
    {
      throw(AskapError("Called base class deconvolver"));
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::dirty(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsDirty(term);
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::psf(const uInt term)
    {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      return itsPsf(term);
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setWeight(Array<T> weight, const uInt term) {
      ASKAPCHECK(term<itsNumberTerms, "Term " << term << " greater than allowed " << itsNumberTerms);
      ASKAPCHECK(term>=0, "Term " << term << " less than zero");
      this->itsWeight(term)=weight.nonDegenerate();
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
      ASKAPLOG_INFO_STR(decbaselogger, "Initialising weight images"); 

      // Always check shapes on initialise
      this->validateShapes();

    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::finalise() {
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateResiduals(Vector<Array<T> >& model)
    {
      ASKAPCHECK(model.shape()==itsNumberTerms, "Number of terms in model " << model.shape()
		 << " not same as number of terms specified "
		 << itsNumberTerms);

      for (uInt term=0;term<itsNumberTerms;term++) {
	Array<FT> xfr;
	xfr.resize(psf(term).shape());
	casa::setReal(xfr, psf(term));
	scimath::fft2d(xfr, true);
	Array<FT> work;
	// Find residuals for current model model
	work.resize(model(term).shape());
	work.set(FT(0.0));
	casa::setReal(work, model(term));
	scimath::fft2d(work, true);
	work=xfr*work;
	scimath::fft2d(work, false);
	this->dirty(term)=this->dirty(term)-real(work);
      }
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateResiduals(Array<T> & model)
    {
      Vector<Array<T> > modelVec(1);
      modelVec(0)=model;
      updateResiduals(modelVec);
    }

    template<class T, class FT>
    uInt DeconvolverBase<T,FT>::auditMemory(Vector<casa::Array<T> >& vecArray) {
      uInt memory=0;
      for (uInt term=0;term<vecArray.nelements();term++) {
        memory+=sizeof(T)*vecArray(term).nelements();
      }
      return memory;
    }

    template<class T, class FT>
    uInt DeconvolverBase<T,FT>::auditMemory(Vector<casa::Array<FT> >& vecArray) {
      uInt memory=0;
      for (uInt term=0;term<vecArray.nelements();term++) {
        memory+=sizeof(FT)*vecArray(term).nelements();
      }
      return memory;
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::auditAllMemory() {
      ASKAPLOG_DEBUG_STR(decbaselogger, "Dirty images  " << auditMemory(itsDirty));
      ASKAPLOG_DEBUG_STR(decbaselogger, "PSFs          " << auditMemory(itsPsf));
      ASKAPLOG_DEBUG_STR(decbaselogger, "Models        " << auditMemory(itsModel));
      ASKAPLOG_DEBUG_STR(decbaselogger, "Weight images " << auditMemory(itsWeight));
    }
  } // namespace synthesis
  
} // namespace askap


