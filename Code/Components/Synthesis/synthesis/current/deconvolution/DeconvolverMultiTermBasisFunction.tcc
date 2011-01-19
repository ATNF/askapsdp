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
ASKAP_LOGGER(decmtbflogger, ".deconvolution.multitermbasisfunction");

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <string>

#include <deconvolution/DeconvolverMultiTermBasisFunction.h>

#include <deconvolution/MultiScaleBasisFunction.h>

#include <measurementequation/SynthesisParamsHelper.h>

namespace askap {
  
  namespace synthesis {
    
    /// @brief Class for a deconvolver based on the BasisFunction Clean
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a residual image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverMultiTermBasisFunction<Double, DComplex>
    /// @ingroup Deconvolver
    
    template<class T, class FT>
    DeconvolverMultiTermBasisFunction<T,FT>::DeconvolverMultiTermBasisFunction(Vector<Array<T> >& dirty, Vector<Array<T> >& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf)
    {
    };
    
    template<class T, class FT>
    DeconvolverMultiTermBasisFunction<T,FT>::DeconvolverMultiTermBasisFunction(Array<T>& dirty, Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf)
    {
    };
    
    template<class T, class FT>
    DeconvolverMultiTermBasisFunction<T,FT>::~DeconvolverMultiTermBasisFunction() {
    };
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::setBasisFunction(boost::shared_ptr<BasisFunction<T> > bf) {
      itsBasisFunction=bf;
    };
    
    template<class T, class FT>
    boost::shared_ptr<BasisFunction<T> > DeconvolverMultiTermBasisFunction<T,FT>::basisFunction() {
      return itsBasisFunction;
    };
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::configure(const LOFAR::ParameterSet& parset)
    {        
      this->control()->configure(parset);
      
      // Make the basis function
      {
	std::vector<float> defaultScales(3);
	defaultScales[0]=0.0;
	defaultScales[1]=10.0;
	defaultScales[2]=30.0;
	std::vector<float> scales=parset.getFloatVector("scales", defaultScales);
	
	ASKAPLOG_INFO_STR(decmtbflogger, "Constructing Multiscale basis function with scales " << scales);
        Bool orthogonal=parset.getBool("orthogonal", "false");

	itsBasisFunction = BasisFunction<Float>::ShPtr(new MultiScaleBasisFunction<Float>(scales,
                                                                                          orthogonal));
      }
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::finalise()
    {
      this->updateResiduals(this->itsModel);
      
      for (uInt base=0;base<itsTermBaseFlux.nelements();base++) {
	for (uInt term=0;term<itsTermBaseFlux(base).nelements();term++) {
	  ASKAPLOG_INFO_STR(decmtbflogger, "   Term(" <<term << "), Base(" << base << "): Flux = " << itsTermBaseFlux(term)(base));
	}
      }
      
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();

      ASKAPLOG_INFO_STR(decmtbflogger, "Initialising Basis Function deconvolver");

      Int psfWidth=this->model().shape()(0);
      
      // Only use the specified psfWidth if it makes sense
      if((this->control()->psfWidth()>0)&&(this->control()->psfWidth()<psfWidth)) {
	psfWidth=this->control()->psfWidth();
	ASKAPLOG_INFO_STR(decmtbflogger, "Using subregion of Psf : size " << psfWidth
			  << " pixels");
      }
      IPosition subPsfShape(2, psfWidth, psfWidth);
      
      // Initialise the basis function. Then we can initialise the residual and psf
      // convolutions
      this->itsBasisFunction->initialise(this->model(0).shape());
      initialiseResidual();

      // Use a smaller size for the psfs if specified. Initialise the
      // PSF convolutions
      this->itsBasisFunction->initialise(subPsfShape);
      initialisePSF();

      // Initialise the model
      for (uInt term=0;term<this->itsModel.nelements();term++) {
	this->model(term).set(T(0.0));
      }
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialiseResidual()
    {
      ASKAPCHECK(this->itsBasisFunction, "Basis function not initialised");
      
      ASKAPLOG_INFO_STR(decmtbflogger, "Shape of basis functions "
			<< this->itsBasisFunction->basisFunction().shape());
      
      this->state()->resetInitialObjectiveFunction();
      
      ASKAPLOG_INFO_STR(decmtbflogger, "Calculating cache of images");
      
      uInt nBases(this->itsBasisFunction->numberBases());
      uInt nTerms(this->itsResidual.nelements());

      itsResidualBasisFunction.resize(nTerms);
      
      // Calculate transform of basis function [nx,ny,nbases]
      Cube<FT> basisFunctionFFT(this->itsBasisFunction->basisFunction().shape());
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);
      
      // Calculate residuals convolved with bases [nx,ny][nterms][nbases]
      for (uInt base=0;base<nBases;base++) {
	itsResidualBasisFunction(base).resize(nBases);
      
	// Calculate transform of residual images [nx,ny,nterms]
	ASKAPLOG_INFO_STR(decmtbflogger,
			  "Calculating convolutions of residual image with basis functions");
	for (uInt term=0;term<this->itsNumberTerms;term++) {
	  Array<FT> residualFFT(this->residual(term).shape().nonDegenerate());
	  residualFFT.set(FT(0.0));
	  casa::setReal(residualFFT, this->residual(term).nonDegenerate());
	  scimath::fft2d(residualFFT, true);
      
	  Array<FT> work(this->residual(term).nonDegenerate().shape());
      
	  ASKAPASSERT(basisFunctionFFT.xyPlane(base).nonDegenerate().shape().conform(residualFFT.nonDegenerate().shape()));
	  work=conj(basisFunctionFFT.xyPlane(base).nonDegenerate())*residualFFT.nonDegenerate();
	  scimath::fft2d(work, false);
	
	  // basis function * residual
	  ASKAPLOG_INFO_STR(decmtbflogger, "Basis function(" << term << "," << base
			    << ") * Residual: max = " << max(real(work))
			    << " min = " << min(real(work)));
	
	  itsResidualBasisFunction(base)(term)=real(work);
	}
      }
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialisePSF()
    {
      ASKAPCHECK(this->itsBasisFunction, "Basis function not initialised");
      
      // For the psf convolutions, we only need a small part of the
      // basis functions so we recalculate for that size
      Int psfWidth=this->model(0).shape()(0);
      
      // Only use the specified psfWidth if it makes sense
      if((this->control()->psfWidth()>0)&&(this->control()->psfWidth()<psfWidth)) {
	psfWidth=this->control()->psfWidth();
	ASKAPLOG_INFO_STR(decmtbflogger, "Using subregion of Psf : size " << psfWidth
			  << " pixels");
      }
      
      IPosition subPsfShape(2, psfWidth, psfWidth);
      
      Array<FT> work(subPsfShape);
      
      ASKAPLOG_INFO_STR(decmtbflogger, "Shape of basis functions "
			<< this->itsBasisFunction->basisFunction().shape());
      
      IPosition stackShape(this->itsBasisFunction->basisFunction().shape());
      
      uInt nBases(this->itsBasisFunction->numberBases());
      uInt nTerms(this->itsResidual.nelements());

      // Now transform the basis functions. These may be a different size from
      // those in initialiseResidual so we don't keep either
      Cube<FT> basisFunctionFFT(this->itsBasisFunction->basisFunction().shape());
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);

      itsTermBaseFlux.resize(nBases);
      for(uInt base=0;base<nBases;base++) {
	itsTermBaseFlux(base).resize(nTerms);
	itsTermBaseFlux(base)=0.0;
      }

      // Calculate XFR for the subsection only
      Array<FT> subXFR(subPsfShape);
       
      for (uInt base=0;base<nBases;base++) {
	itsPSFBasis(base).resize(nTerms);
	for (uInt base1=base;base1<nBases;base1++) {
	  itsPSFBasisBasis(base,base1).resize(nTerms);
	}
      }

     // Loop over all terms, calculating transfer function
      for (uInt term=0;term<this->itsNumberTerms;term++) {

	uInt nx(this->psf(term).shape()(0));
	uInt ny(this->psf(term).shape()(1));
	
	IPosition subPsfStart(2,nx/2-psfWidth/2,ny/2-psfWidth/2);
	IPosition subPsfEnd(2,nx/2+psfWidth/2-1,ny/2+psfWidth/2-1);
	IPosition subPsfStride(2,1,1);
	
	Slicer subPsfSlicer(subPsfStart, subPsfEnd, subPsfStride, Slicer::endIsLast);
	
	casa::IPosition minPos;
	casa::IPosition maxPos;
	T minVal, maxVal;
	ASKAPLOG_INFO_STR(logger, "Validating subsection of PSF");
	casa::minMax(minVal, maxVal, minPos, maxPos, this->psf(term).nonDegenerate()(subPsfSlicer));
	ASKAPLOG_INFO_STR(logger, "Maximum of Psf(" << term << ") = " << maxVal << " at " << maxPos);
	ASKAPLOG_INFO_STR(logger, "Minimum of Psf(" << term << ") = " << minVal << " at " << minPos);
	this->itsPeakPSFVal(term) = maxVal;
	this->itsPeakPSFPos(term)(0)=maxPos(0);
	this->itsPeakPSFPos(term)(1)=maxPos(1);
	
	casa::setReal(subXFR, this->psf(term).nonDegenerate()(subPsfSlicer));
	scimath::fft2d(subXFR, true);
      
	// Now we have all the ingredients to calculate the convolutions
	// of basis function with psf's, etc. for bach basis
	ASKAPLOG_INFO_STR(decmtbflogger, "Calculating convolutions of Psf(" << term << ") with basis functions");
	for(uInt base=0;base<nBases;base++) {
	  ASKAPASSERT(basisFunctionFFT.xyPlane(base).nonDegenerate().shape().conform(subXFR.shape()));
	  work=conj(basisFunctionFFT.xyPlane(base).nonDegenerate())*subXFR;
	  scimath::fft2d(work, false);
	  ASKAPLOG_INFO_STR(decmtbflogger, "Basis function(" << base << ") * PSF( " << term << "): max = "
			    << max(real(work)) << " min = " << min(real(work)));
	  itsPSFBasis(base)(term)=real(work);

	  // Now do the cross terms
	  for (uInt base1=base;base1<nBases;base1++) {
	    work=conj(basisFunctionFFT.xyPlane(base).nonDegenerate())*basisFunctionFFT.xyPlane(base1).nonDegenerate()*subXFR;
	    scimath::fft2d(work, false);
	    ASKAPLOG_INFO_STR(decmtbflogger, "Base (" << base << ") * Base(" << base1 << " * PSF( " << term << "): max = "
			    << max(real(work)) << " min = " << min(real(work)));

	    itsPSFBasisBasis(base,base1)(term)=real(work);
	  }
	}
      }
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::calculateTermCoupling(Vector<Array<T> > & psfVec)
    {

      ASKAPCHECK(this->itsBasisFunction, "Basis function not initialised");
      
      // For the psf convolutions, we only need a small part of the
      // basis functions so we recalculate for that size
      Int psfWidth=this->model(0).shape()(0);
      
      // Only use the specified psfWidth if it makes sense
      if((this->control()->psfWidth()>0)&&(this->control()->psfWidth()<psfWidth)) {
	psfWidth=this->control()->psfWidth();
	ASKAPLOG_INFO_STR(decmtbflogger, "Using subregion of Psf : size " << psfWidth
			  << " pixels");
      }
      
      IPosition subPsfShape(2, psfWidth, psfWidth);
      
      Array<FT> work(subPsfShape);
      
      ASKAPLOG_INFO_STR(decmtbflogger, "Shape of basis functions "
			<< this->itsBasisFunction->basisFunction().shape());
      
      // Now transform the basis functions. These may be a different size from
      // those in initialiseResidual so we don't keep either
      Cube<FT> basisFunctionFFT(this->itsBasisFunction->basisFunction().shape());
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);

      uInt nx(this->psf().shape()(0));
      uInt ny(this->psf().shape()(1));
      
      IPosition subPsfStart(2,nx/2-psfWidth/2,ny/2-psfWidth/2);
      IPosition subPsfEnd(2,nx/2+psfWidth/2-1,ny/2+psfWidth/2-1);
      IPosition subPsfStride(2,1,1);
      Slicer subPsfSlicer(subPsfStart, subPsfEnd, subPsfStride, Slicer::endIsLast);

      uInt nBases(this->itsBasisFunction->numberBases());
      uInt nTerms(this->itsResidual.nelements());

      // For each base, calculate the coupling between different terms
      // Calculate XFR for the subsection only
      ASKAPCHECK(psfVec.nelements()==(2*nTerms-1), "Insufficient PSFs to calculate term-term coupling");

      Vector<Array<FT> > subXFRVec(psfVec.nelements());
    
      // Transform all the PSFs: note that we need all the PSF terms up to
      // 2*N-1 so that we can use the prescription from Urvashi:
      // B(i)*B(j)=B(0)*B(i+j)
      // We need all of these upfront so we can do the cross terms
      for (uInt term=0;term<(2*nTerms-1);term++) {
	subXFRVec(term).resize(subPsfShape);
	casa::setReal(subXFRVec(term), psfVec(term).nonDegenerate()(subPsfSlicer));
	scimath::fft2d(subXFRVec(term), true);
      }
      
      // Now we can calculate the peak of the cross terms. These are all we need
      // to get the term coupling matrix.
      ASKAPLOG_INFO_STR(decmtbflogger, "Calculating convolutions of Psfs with basis functions");
      this->itsCouplingMatrix.resize(nBases);
      for (uInt base=0;base<nBases;base++) {
	this->itsCouplingMatrix(base).resize(nTerms,nTerms);
	for (uInt term1=0;term1<nTerms;term1++) {
	  for (uInt term2=term1;term2<nTerms;term2++) {
	    ASKAPASSERT(basisFunctionFFT.xyPlane(base).nonDegenerate().shape().conform(subXFRVec(term1).shape()));
	    ASKAPASSERT(basisFunctionFFT.xyPlane(base).nonDegenerate().shape().conform(subXFRVec(term2).shape()));
	    work=conj(basisFunctionFFT.xyPlane(base).nonDegenerate())*subXFRVec(term1+term2)*conj(subXFRVec(0));
	    scimath::fft2d(work, false);
	    itsCouplingMatrix(base)(term1,term2)=max(real(work));
	    itsCouplingMatrix(base)(term2,term1)=itsCouplingMatrix(base)(term1,term2);
	  }
	}
      }

      // Invert the coupling matrices and check for correctness
      this->itsInverseCouplingMatrix.resize(nBases);
      for (uInt base=0;base<nBases;base++) {
	ASKAPLOG_INFO_STR(decmtbflogger, "Coupling matrix(" << base << ")=" << this->itsCouplingMatrix(base));
	invertSymPosDef(this->itsInverseCouplingMatrix(base), this->itsDetCouplingMatrix(base), this->itsCouplingMatrix(base));
	ASKAPLOG_INFO_STR(decmtbflogger, "Coupling matrix determinant(" << base << ") = " << this->itsDetCouplingMatrix(base));
	ASKAPLOG_INFO_STR(decmtbflogger, "Inverse coupling matrix(" << base << ")=" << this->itsInverseCouplingMatrix(base));
	// Check that the inverse really is an inverse.
	Matrix<T> identity(this->itsCouplingMatrix(base).shape());
	identity.set(T(0.0));
	uInt nRows(this->itsCouplingMatrix(base).nrow());
	uInt nCols(this->itsCouplingMatrix(base).ncolumn());
	for (uInt row=0;row<nRows;row++) {
	  for (uInt col=0;col<nCols;col++) {
	    identity(row,col)=sum(this->itsCouplingMatrix(base).row(row)*this->itsInverseCouplingMatrix(base).column(col));
	  }
	}
	ASKAPLOG_INFO_STR(decmtbflogger, "Coupling matrix * inverse " << identity);
      }
    }

    template<class T, class FT>
    bool DeconvolverMultiTermBasisFunction<T,FT>::deconvolve()
    {
      
      this->initialise();
      
      uInt nBases(this->itsBasisFunction->numberBases());

      ASKAPCHECK(this->itsCouplingMatrix.nelements()==nBases, "Term-term coupling matrix not yet initialised")

      ASKAPLOG_INFO_STR(decmtbflogger, "Performing Multi-Term BasisFunction CLEAN for "
			<< this->control()->targetIter() << " iterations");
      do {
	this->oneIteration();
	this->monitor()->monitor(*(this->state()));
	this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));
      
      ASKAPLOG_INFO_STR(decmtbflogger, "Performed Multi-Term BasisFunction CLEAN for "
			<< this->state()->currentIter() << " iterations");
      
      ASKAPLOG_INFO_STR(decmtbflogger, this->control()->terminationString());
      
      this->finalise();
      
      return True;
    }
    
    // This contains the heart of the Multi-Term BasisFunction Clean algorithm
    template<class T, class FT>
    bool DeconvolverMultiTermBasisFunction<T,FT>::oneIteration()
    {

      uInt nBases(this->itsBasisFunction->numberBases());
      uInt nTerms(this->itsResidual.nelements());

      // Find peak in residual image cube. This cube is full sized.
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal(0.0), maxVal(0.0);

      casa::IPosition tMinPos;
      casa::IPosition tMaxPos;
      T tMinVal(0.0), tMaxVal(0.0);

      uInt optimumBase(0);
      casa::IPosition absPeakPos;
      T absPeakVal(0.0);

      // Find the base having the peak value in term=0
      // Here the weighted mask is used as a weight in the determination
      // of the maximum i.e. it finds the max in mask . residual. The values
      // returned are without the mask
      for(uInt base=0;base<nBases;base++) {
	uInt term=0;
	bool isMasked(this->itsMask(term).shape().nonDegenerate().conform(this->itsResidualBasisFunction(base)(term).shape()));
	if(isMasked) {
	  casa::minMaxMasked(minVal, maxVal, minPos, maxPos,
			     this->itsResidualBasisFunction(base)(term), this->itsMask(term).nonDegenerate());
	  minVal=this->itsResidualBasisFunction(base)(term)(minPos);
	  maxVal=this->itsResidualBasisFunction(base)(term)(maxPos);
	}
	else {
	  casa::minMax(minVal, maxVal, minPos, maxPos, this->itsResidualBasisFunction(base)(term));
	}
	if(minVal<tMinVal) {
	  tMinVal=minVal;
	  tMinPos=minPos;
	}
	if(maxVal>tMaxVal) {
	  tMaxVal=maxVal;
	  tMinPos=minPos;
	}
	if(abs(tMinVal)>absPeakVal) {
	  optimumBase=base;
	  absPeakVal=abs(tMinVal);
	  absPeakPos=tMinPos;
	}
	if(abs(tMaxVal)>absPeakVal) {
	  optimumBase=base;
	  absPeakVal=abs(tMaxVal);
	  absPeakPos=tMaxPos;
	}
      }

      // Find the vector of values for the optimum base
      Vector<T> coupledPeakValues(nTerms);
      for (uInt term=0;term<nTerms;term++) {
	coupledPeakValues(term)=itsResidualBasisFunction(optimumBase)(term)(absPeakPos);
      }

      // Apply inverse of term coupling matrix to get the true decoupled values
      Vector<T> peakValues(nTerms);
      peakValues=findCoefficients(this->itsInverseCouplingMatrix(optimumBase), coupledPeakValues);
      // We want the absolute peak value after decoupling
      absPeakVal=max(abs(peakValues));

      // Report on progress
      if(this->state()->initialObjectiveFunction()==0.0) {
	this->state()->setInitialObjectiveFunction(abs(absPeakVal));
      }
      this->state()->setPeakResidual(abs(absPeakVal));
      this->state()->setObjectiveFunction(abs(absPeakVal));
      this->state()->setTotalFlux(sum(this->model(0)));
      
      // Now we adjust model and residual for this component
      const casa::IPosition residualShape(this->itsResidualBasisFunction(optimumBase)(0).shape());
      const casa::IPosition psfShape(this->itsPSFBasis(optimumBase)(0).shape());
      
      casa::IPosition residualStart(2,0), residualEnd(2,0), residualStride(2,1);
      casa::IPosition psfStart(2,0), psfEnd(2,0), psfStride(2,1);
      
      const casa::IPosition modelShape(this->model(0).shape());

      const casa::uInt modelNdim(this->model().shape().size());
      casa::IPosition modelStart(modelNdim,0), modelEnd(modelNdim,0), modelStride(modelNdim,1);
      
      // Wrangle the start, end, and shape into consistent form. It took me 
      // quite a while to figure this out (slow brain day) so it may be
      // that there are some edge cases for which it fails.
      for (uInt dim=0;dim<2;dim++) {
	residualStart(dim)=max(0, Int(absPeakPos(dim)-psfShape(dim)/2));
	residualEnd(dim)=min(Int(absPeakPos(dim)+psfShape(dim)/2-1), Int(residualShape(dim)-1));
	// Now we have to deal with the PSF. Here we want to use enough of the
	// PSF to clean the residual image.
	psfStart(dim)=max(0, Int(this->itsPeakPSFPos(0)(dim)-(absPeakPos(dim)-residualStart(dim))));
	psfEnd(dim)=min(Int(this->itsPeakPSFPos(0)(dim)-(absPeakPos(dim)-residualEnd(dim))),
			Int(psfShape(dim)-1));
	
	modelStart(dim)=residualStart(dim);
	modelEnd(dim)=residualEnd(dim);
      }
      casa::Slicer psfSlicer(psfStart, psfEnd, psfStride, Slicer::endIsLast);
      casa::Slicer residualSlicer(residualStart, residualEnd, residualStride, Slicer::endIsLast);
      casa::Slicer modelSlicer(modelStart, modelEnd, modelStride, Slicer::endIsLast);

      // Add to model
      // We loop over all terms for the optimum base and ignore those terms with no flux
      for (uInt term=0;term<nTerms;term++) {
	if(abs(peakValues(term))>0.0) {
	  this->model(term)(modelSlicer).nonDegenerate() = this->model(term)(modelSlicer).nonDegenerate()
	    + this->control()->gain()*peakValues(term)*
	    Cube<T>(this->itsBasisFunction->basisFunction()).xyPlane(optimumBase)(psfSlicer).nonDegenerate();
	  this->itsTermBaseFlux(optimumBase)(term)+=this->control()->gain()*peakValues(term);
	}
      }	
      
      // Subtract PSFs, including base-base crossterms
      for (uInt term=0;term<nTerms;term++) {
	if(abs(peakValues(term))>0.0) {
	  for (uInt base=0;base<nBases;base++) {
	    this->itsResidualBasisFunction(base)(term).nonDegenerate() =
	      this->itsResidualBasisFunction(base)(term).nonDegenerate()
	      - this->control()->gain()*peakValues(term)*this->itsPSFBasis(base)(term).nonDegenerate();
	    if(base<optimumBase) {
	      this->itsResidualBasisFunction(base)(term).nonDegenerate() =
		this->itsResidualBasisFunction(base)(term).nonDegenerate()
		- this->control()->gain()*peakValues(term) *
		this->itsPSFBasisBasis(base,optimumBase)(term).nonDegenerate();
	    }
	    if(base>optimumBase) {
	      this->itsResidualBasisFunction(base)(term).nonDegenerate() =
		this->itsResidualBasisFunction(base)(term).nonDegenerate()
		- this->control()->gain()*peakValues(term) *
		this->itsPSFBasisBasis(optimumBase,base)(term).nonDegenerate();
	    }
	  }
	}
      }
      return True;
    }
    
    template<class T, class FT>
    Vector<T> DeconvolverMultiTermBasisFunction<T, FT>::findCoefficients(const Matrix<Double>& invCoupling,
								const Vector<T>& peakValues)
    {
      uInt nRows(invCoupling.nrow());
      uInt nCols(invCoupling.ncolumn());
      Vector<T> coefficients(nRows);
      for (uInt row=0;row<nRows;row++) {
	coefficients(row)=T(0.0);
	for (uInt col=0;col<nCols;col++) {
	  coefficients(row)+=T(invCoupling(row,col))*peakValues(col);
	}
      }
      return coefficients;
    }

  }
}
// namespace synthesis 
// namespace askap
