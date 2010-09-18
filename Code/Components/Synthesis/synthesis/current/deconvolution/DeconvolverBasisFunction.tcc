/// @file
/// @brief Class for a deconvolver based on CLEANing with basis functions.
/// @details This concrete class defines a deconvolver used to estimate an
/// image from a residual image, psf optionally using a mask and a weights image.
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
ASKAP_LOGGER(decbflogger, ".deconvolution.basisfunction");

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <string>

#include <deconvolution/DeconvolverBasisFunction.h>

#include <deconvolution/MultiScaleBasisFunction.h>

namespace askap {
  
  namespace synthesis {
    
    /// @brief Class for a deconvolver based on the BasisFunction Clean
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a residual image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverBasisFunction<Double, DComplex>
    /// @ingroup Deconvolver
    
    template<class T, class FT>
    DeconvolverBasisFunction<T,FT>::DeconvolverBasisFunction(Array<T>& dirty, Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf), itsUseCrossTerms(true)
    {
    };
    
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
    void DeconvolverBasisFunction<T,FT>::configure(const LOFAR::ParameterSet& parset)
    {        
      this->control()->configure(parset);
      
      // Make the basis function
      {
	std::vector<float> defaultScales(3);
	defaultScales[0]=0.0;
	defaultScales[1]=10.0;
	defaultScales[2]=30.0;
	std::vector<float> scales=parset.getFloatVector("scales", defaultScales);
	
	ASKAPLOG_INFO_STR(decbflogger, "Constructing Multiscale basis function with scales " << scales);
	itsBasisFunction = BasisFunction<Float>::ShPtr(new MultiScaleBasisFunction<Float>(scales));
      }
      itsUseCrossTerms=parset.getBool("usecrossterms", true);
      
    }
    
    template<class T, class FT>
    void DeconvolverBasisFunction<T,FT>::finalise()
    {
      DeconvolverBase<T, FT>::updateResiduals(this->model());

      uInt nScales(this->itsBasisFunction->numberTerms());
      IPosition l1Shape(3, this->model().shape()(0), this->model().shape()(1), nScales);
      
      Array<T> ones(this->itsL1image.shape());
      ones.set(T(1.0));
      T l0Norm(sum(ones(abs(this->itsL1image)>T(0.0))));
      T l1Norm(sum(abs(this->itsL1image)));
      ASKAPLOG_INFO_STR(decbflogger, "L0 norm = " << l0Norm << ", L1 norm   = " << l1Norm
			<< ", Flux = " << sum(this->model()));
      
      for (uInt scale=0;scale<itsScaleFlux.nelements();scale++) {
	ASKAPLOG_INFO_STR(decbflogger, "   Scale " << scale << " Flux = " << itsScaleFlux(scale));
      }
      
    }
    
    template<class T, class FT>
    void DeconvolverBasisFunction<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();
      initialiseResidual();
      initialisePSF();

      uInt nScales(this->itsBasisFunction->numberTerms());

      IPosition l1Shape(3, this->model().shape()(0), this->model().shape()(1), nScales);
      this->itsL1image.resize(l1Shape);
      this->itsL1image.set(0.0);

      this->itsModel=this->itsBackground.copy();

    }
    
    template<class T, class FT>
    void DeconvolverBasisFunction<T,FT>::initialiseResidual()
    {
      
      ASKAPCHECK(this->itsBasisFunction, "Basis function not initialised");
      
      this->state()->resetInitialObjectiveFunction();
      
      ASKAPLOG_INFO_STR(decbflogger, "Calculating cache of images");
      
      // Now calculate the convolutions of the residual images and PSFs 
      // with the basis functions
      
      this->itsBasisFunction->initialise(this->itsModel.shape());
      
      ASKAPLOG_INFO_STR(decbflogger, "Shape of basis functions "
			<< this->itsBasisFunction->basisFunction().shape());
      
      IPosition stackShape(this->itsBasisFunction->basisFunction().shape());
      
      itsResidualBasisFunction.resize(stackShape);
      
      Array<FT> bfPlane(this->model().shape());
      
      Cube<FT> basisFunctionFFT(this->itsBasisFunction->basisFunction().shape());
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);
      
      Array<FT> residualFFT(this->itsResidual.shape().nonDegenerate());
      residualFFT.set(FT(0.0));
      casa::setReal(residualFFT, this->itsResidual.nonDegenerate());
      scimath::fft2d(residualFFT, true);
      
      Array<FT> work(this->model().nonDegenerate().shape());
      ASKAPLOG_INFO_STR(decbflogger,
			"Calculating convolutions of residual image with basis functions");
      
      for (uInt term=0;term<this->itsBasisFunction->numberTerms();term++) {
	
	ASKAPASSERT(basisFunctionFFT.xyPlane(term).nonDegenerate().shape().conform(residualFFT.nonDegenerate().shape()));
        work=conj(basisFunctionFFT.xyPlane(term).nonDegenerate())*residualFFT.nonDegenerate();
        scimath::fft2d(work, false);
	
        // basis function * residual
        ASKAPLOG_INFO_STR(decbflogger, "Basis function(" << term
			  << ") * Residual: max = " << max(real(work))
			  << " min = " << min(real(work)));
	
        Cube<T>(itsResidualBasisFunction).xyPlane(term)=real(work);
      }
    }
    
    template<class T, class FT>
    void DeconvolverBasisFunction<T,FT>::initialisePSF()
    {
      // For the psf convolutions, we only need a small part of the
      // basis functions so we recalculate for that size
      Int psfWidth=this->itsModel.shape()(0);
      
      // Only use the specified psfWidth if it makes sense
      if((this->control()->psfWidth()>0)&&(this->control()->psfWidth()<psfWidth)) {
	psfWidth=this->control()->psfWidth();
	ASKAPLOG_INFO_STR(decbflogger, "Using subregion of PSF : size " << psfWidth
			  << " pixels");
      }
      
      IPosition subPsfShape(2, psfWidth, psfWidth);
      this->itsBasisFunction->initialise(subPsfShape);
      
      Array<FT> work(subPsfShape);
      
      ASKAPLOG_INFO_STR(decbflogger, "Shape of basis functions "
			<< this->itsBasisFunction->basisFunction().shape());
      
      IPosition stackShape(this->itsBasisFunction->basisFunction().shape());
      
      // Now transform the basis functions
      Cube<FT> basisFunctionFFT(this->itsBasisFunction->basisFunction().shape());
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);
      
      itsPSFBasisFunction.resize(stackShape);
      
      itsScaleFlux.resize(stackShape(2));
      itsScaleFlux.set(T(0));
      
      // Calculate XFR for the subsection only
      Array<FT> subXFR(subPsfShape);
      
      uInt nx(this->itsPSF.shape()(0));
      uInt ny(this->itsPSF.shape()(1));
      
      IPosition subPsfStart(2,nx/2-psfWidth/2,ny/2-psfWidth/2);
      IPosition subPsfEnd(2,nx/2+psfWidth/2-1,ny/2+psfWidth/2-1);
      IPosition subPsfStride(2,1,1);
      
      Slicer subPsfSlicer(subPsfStart, subPsfEnd, subPsfStride, Slicer::endIsLast);
      
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      ASKAPLOG_INFO_STR(logger, "Validating subsection of PSF");
      casa::minMax(minVal, maxVal, minPos, maxPos, this->itsPSF.nonDegenerate()(subPsfSlicer));
      ASKAPLOG_INFO_STR(logger, "Maximum of PSF = " << maxVal << " at " << maxPos);
      ASKAPLOG_INFO_STR(logger, "Minimum of PSF = " << minVal << " at " << minPos);
      this->itsPeakPSFVal = maxVal;
      this->itsPeakPSFPos(0)=maxPos(0);
      this->itsPeakPSFPos(1)=maxPos(1);
      
      casa::setReal(subXFR, this->itsPSF.nonDegenerate()(subPsfSlicer));
      scimath::fft2d(subXFR, true);
      
      // Now we have all the ingredients to calculate the convolutions
      // of basis function with psf's, etc.
      ASKAPLOG_INFO_STR(decbflogger, "Calculating convolutions of PSFs with basis functions");
      itsPSFScales.resize(this->itsBasisFunction->numberTerms());
      for (uInt term=0;term<this->itsBasisFunction->numberTerms();term++) {
	// basis function * psf
	ASKAPASSERT(basisFunctionFFT.xyPlane(term).nonDegenerate().shape().conform(subXFR.shape()));
	work=conj(basisFunctionFFT.xyPlane(term).nonDegenerate())*subXFR;
	scimath::fft2d(work, false);
	Cube<T>(itsPSFBasisFunction).xyPlane(term)=real(work);
	
	ASKAPLOG_INFO_STR(decbflogger, "Basis function(" << term << ") * PSF: max = " << max(real(work)) << " min = " << min(real(work)));
	
	itsPSFScales(term)=max(real(work));
      }
      
      if(this->itsUseCrossTerms) {
	ASKAPLOG_INFO_STR(decbflogger, "Calculating double convolutions of PSF with basis functions");
	IPosition crossTermsShape(4, psfWidth, psfWidth,
				  this->itsBasisFunction->numberTerms(),
				  this->itsBasisFunction->numberTerms());
	ASKAPLOG_INFO_STR(decbflogger, "Shape of cross terms " << crossTermsShape);
	itsPSFCrossTerms.resize(crossTermsShape);
	IPosition crossTermsStart(4,0);
	IPosition crossTermsEnd(crossTermsShape-1);
	IPosition crossTermsStride(4,1);
	
	Array<FT> crossTermsPSFFFT(crossTermsShape);
	crossTermsPSFFFT.set(T(0));
	
	for (uInt term=0;term<this->itsBasisFunction->numberTerms();term++) {
	  crossTermsStart(2)=term;
	  crossTermsEnd(2)=term;
	  for (uInt term1=0;term1<this->itsBasisFunction->numberTerms();term1++) {
	    crossTermsStart(3)=term1;
	    crossTermsEnd(3)=term1;
	    casa::Slicer crossTermsSlicer(crossTermsStart, crossTermsEnd, crossTermsStride, Slicer::endIsLast);
	    crossTermsPSFFFT(crossTermsSlicer).nonDegenerate()=
	      basisFunctionFFT.xyPlane(term).nonDegenerate()*
	      conj(basisFunctionFFT.xyPlane(term1)).nonDegenerate()*subXFR;
	  }
	  
	}
	this->itsCouplingMatrix.resize(itsBasisFunction->numberTerms(), itsBasisFunction->numberTerms());
	scimath::fft2d(crossTermsPSFFFT, true);
	this->itsPSFCrossTerms=real(crossTermsPSFFFT)/T(crossTermsShape(0)*crossTermsShape(1));
	for (uInt term=0;term<this->itsBasisFunction->numberTerms();term++) {
	  crossTermsStart(2)=term;
	  crossTermsEnd(2)=term;
	  for (uInt term1=0;term1<this->itsBasisFunction->numberTerms();term1++) {
	    crossTermsStart(3)=term1;
	    crossTermsEnd(3)=term1;
	    casa::Slicer crossTermsSlicer(crossTermsStart, crossTermsEnd, crossTermsStride, Slicer::endIsLast);
	    casa::minMax(minVal, maxVal, minPos, maxPos, this->itsPSFCrossTerms(crossTermsSlicer));
	    //	    ASKAPLOG_INFO_STR(decbflogger, "Basis function(" << term << ") * Basis function(" << term1
	    //			      << ") * PSF: max = " << maxVal << " min = " << minVal);
	    this->itsCouplingMatrix(term, term1) = maxVal;
	  }
	}
	ASKAPLOG_INFO_STR(decbflogger, "Coupling matrix " << this->itsCouplingMatrix);
	this->itsInverseCouplingMatrix.resize(this->itsCouplingMatrix.shape());
	invertSymPosDef(this->itsInverseCouplingMatrix, this->itsDetCouplingMatrix, this->itsCouplingMatrix);
	ASKAPLOG_INFO_STR(decbflogger, "Coupling matrix determinant " << this->itsDetCouplingMatrix);
	ASKAPLOG_INFO_STR(decbflogger, "Inverse coupling matrix " << this->itsInverseCouplingMatrix);
      }
    }
    
    template<class T, class FT>
    bool DeconvolverBasisFunction<T,FT>::deconvolve()
    {
      
      this->initialise();
      
      ASKAPLOG_INFO_STR(decbflogger, "Performing BasisFunction CLEAN for "
			<< this->control()->targetIter() << " iterations");
      do {
	this->oneIteration();
	this->monitor()->monitor(*(this->state()));
	this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));
      
      ASKAPLOG_INFO_STR(decbflogger, "Performed BasisFunction CLEAN for "
			<< this->state()->currentIter() << " iterations");
      
      ASKAPLOG_INFO_STR(decbflogger, this->control()->terminationString());
      
      this->finalise();
      
      return True;
    }
    
    // This contains the heart of the BasisFunction Clean algorithm
    // The residual image and psfs are intrinsically two dimensional
    // but are expanded by projection onto the basis functions
    template<class T, class FT>
    bool DeconvolverBasisFunction<T,FT>::oneIteration()
    {
      
      // Find peak in residual image cube. This cube is full sized.
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal(0.0), maxVal(0.0);
      // Here the weighted mask is used as a weight in the determination
      // of the maximum i.e. it finds the max in mask * residual
      minMaxMaskedScales(minVal, maxVal, minPos, maxPos, this->itsResidualBasisFunction,
			 this->itsWeightedMask, this->itsPSFScales);
      //      ASKAPLOG_INFO_STR(decbflogger, "Maximum =  " << maxVal << " at location " << maxPos);
      //      ASKAPLOG_INFO_STR(decbflogger, "Minimum = " << minVal << " at location " << minPos);
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
      
      uInt optimumPlane(absPeakPos(2));
      
      if(this->state()->initialObjectiveFunction()==0.0) {
	this->state()->setInitialObjectiveFunction(abs(absPeakVal));
      }
      this->state()->setPeakResidual(abs(absPeakVal));
      this->state()->setObjectiveFunction(abs(absPeakVal));
      this->state()->setTotalFlux(sum(this->model()));
      
      // Has this terminated for any reason?
      if(this->control()->terminate(*(this->state()))) {
	return True;
      }
      
      const casa::IPosition residualShape(this->itsResidualBasisFunction.shape());
      const casa::IPosition psfShape(this->itsPSFBasisFunction.shape());
      const casa::uInt ndim(this->itsResidualBasisFunction.shape().size());
      
      casa::IPosition residualStart(ndim,0), residualEnd(ndim,0), residualStride(ndim,1);
      casa::IPosition psfStart(ndim,0), psfEnd(ndim,0), psfStride(ndim,1);
      casa::IPosition psfCrossTermsStart(ndim+1,0), psfCrossTermsEnd(ndim+1,0), psfCrossTermsStride(ndim+1,1);
      
      const casa::uInt modelNdim(this->model().shape().size());
      casa::IPosition modelStart(modelNdim,0), modelEnd(modelNdim,0), modelStride(modelNdim,1);
      
      for (uInt dim=0;dim<2;dim++) {
	// Wrangle the start, end, and shape into consistent form. It took me 
	// quite a while to figure this out (slow brain day) so it may be
	// that there are some edge cases for which it fails.
	residualStart(dim)=max(0, Int(absPeakPos(dim)-psfShape(dim)/2));
	residualEnd(dim)=min(Int(absPeakPos(dim)+psfShape(dim)/2-1), Int(residualShape(dim)-1));
	// Now we have to deal with the PSF. Here we want to use enough of the
	// PSF to clean the residual image.
	psfStart(dim)=max(0, Int(this->itsPeakPSFPos(dim)-(absPeakPos(dim)-residualStart(dim))));
	psfEnd(dim)=min(Int(this->itsPeakPSFPos(dim)-(absPeakPos(dim)-residualEnd(dim))),
			Int(psfShape(dim)-1));
	
	psfCrossTermsStart(dim)=psfStart(dim);
	psfCrossTermsEnd(dim)=psfEnd(dim);
	
	modelStart(dim)=residualStart(dim);
	modelEnd(dim)=residualEnd(dim);
      }
      
      // For the model, we just have to update using the optimum plane
      psfStart(2)=psfEnd(2)=optimumPlane;
      residualStart(2)=residualEnd(2)=optimumPlane;
      casa::Slicer psfSlicer(psfStart, psfEnd, psfStride, Slicer::endIsLast);
      
      // Add to model
      // Note that the model is only two dimensional. We could make it three dimensional
      // and keep the model layers separate
      {
	casa::Slicer modelSlicer(modelStart, modelEnd, modelStride, Slicer::endIsLast);
	
	this->model()(modelSlicer).nonDegenerate() = this->model()(modelSlicer).nonDegenerate()
	  + this->control()->gain()*absPeakVal*
	  this->itsBasisFunction->basisFunction()(psfSlicer).nonDegenerate();
      }
      // Keep track of strengths and locations of components
      IPosition l1PeakPos(3, absPeakPos(0), absPeakPos(1), optimumPlane);
      this->itsL1image(l1PeakPos) = this->itsL1image(l1PeakPos)
	+ this->control()->gain()*abs(absPeakVal);
      this->itsScaleFlux(optimumPlane)+=this->control()->gain()*absPeakVal;
      
      // Subtract PSF for this plane from residual image for the same plane
      {
	casa::Slicer residualSlicer(residualStart, residualEnd, residualStride, Slicer::endIsLast);
	this->itsResidualBasisFunction(residualSlicer).nonDegenerate() =
	  this->itsResidualBasisFunction(residualSlicer).nonDegenerate()
	  - this->control()->gain()*absPeakVal*this->itsPSFBasisFunction(psfSlicer).nonDegenerate();
      }
      if(itsUseCrossTerms) {
	casa::uInt nterms(this->itsResidualBasisFunction.shape()(2));
	for (uInt term=0;term<nterms;term++) {
	  if(term!=optimumPlane) {
	    residualStart(2)=term;
	    residualEnd(2)=term;
	    casa::Slicer residualSlicer(residualStart, residualEnd, residualStride, Slicer::endIsLast);
	    
	    psfCrossTermsStart(2)=optimumPlane;
	    psfCrossTermsEnd(2)=optimumPlane;
	    psfCrossTermsStart(3)=term;
	    psfCrossTermsEnd(3)=term;
	    casa::Slicer psfCrossTermsSlicer(psfCrossTermsStart, psfCrossTermsEnd, psfCrossTermsStride, Slicer::endIsLast);
	    this->itsResidualBasisFunction(residualSlicer).nonDegenerate() =
	      this->itsResidualBasisFunction(residualSlicer).nonDegenerate()
	      - this->control()->gain()*absPeakVal*this->itsPSFCrossTerms(psfCrossTermsSlicer).nonDegenerate();
	  }
	}
      }
      
      return True;
    }
    
    template<class T, class FT>
    void DeconvolverBasisFunction<T, FT>::minMaxMaskedScales(T& minVal, T& maxVal,
							     IPosition& minPos, IPosition& maxPos,
							     const Array<T>& dataArray, 
							     const Array<T>& maskArray,
							     const Vector<T>& psfScale) {
      
      const Cube<T> data(dataArray);
      bool isMasked(maskArray.shape().nonDegenerate().conform(data.xyPlane(0).shape()));

      uInt nScales=data.shape()(2);
      
      Vector<T> sMaxVal(nScales);
      Vector<T> sMinVal(nScales);
      Vector<IPosition> sMinPos(nScales);
      Vector<IPosition> sMaxPos(nScales);
      {
	if(isMasked) {
	  for (uInt scale=0;scale<nScales;scale++) {
	    casa::minMaxMasked(sMinVal(scale), sMaxVal(scale), sMinPos(scale), sMaxPos(scale),
			       data.xyPlane(scale), maskArray.nonDegenerate());
	  }
	}
	else {
	  for (uInt scale=0;scale<nScales;scale++) {
	    casa::minMax(sMinVal(scale), sMaxVal(scale), sMinPos(scale), sMaxPos(scale),
			 data.xyPlane(scale));
	  }
	}
      }
      maxVal=sMaxVal(0)/=sqrt(this->itsPSFScales(0));
      minVal=sMinVal(0)/=sqrt(this->itsPSFScales(0));
      minPos=IPosition(3, sMinPos(0)(0), sMinPos(0)(1), 0);
      maxPos=IPosition(3, sMaxPos(0)(0), sMaxPos(0)(1), 0);
      for (uInt scale=1;scale<nScales;scale++) {
	// Need to scale by (a) the inverse of the psfScale to deal with the
	// loss of gain for the tapering and (b) by sqrt(psfScale) to deal 
	// with the SNR degradation. The latter is similar to the use of 
	// the small scale bias in the MSClean paper.
	sMaxVal(scale)/=sqrt(this->itsPSFScales(scale));
	sMinVal(scale)/=sqrt(this->itsPSFScales(scale));
	if(sMinVal(scale)<=minVal) {
	  minVal=sMinVal(scale);
	  minPos=IPosition(3, sMinPos(scale)(0), sMinPos(scale)(1), scale);
	}
	if(sMaxVal(scale)>=maxVal) {
	  maxVal=sMaxVal(scale);
	  maxPos=IPosition(3, sMaxPos(scale)(0), sMaxPos(scale)(1), scale);
	}
      }
      // If masking (presumably with weights) was done we need to 
      // look up the original values (without the weights). Since this
      // does no harm for the unmasked case, we do the same for 
      // convenience.
      minVal=data.xyPlane(minPos(2))(minPos);
      maxVal=data.xyPlane(maxPos(2))(maxPos);
    }
  } // namespace synthesis 
} // namespace askap
  
  
  
