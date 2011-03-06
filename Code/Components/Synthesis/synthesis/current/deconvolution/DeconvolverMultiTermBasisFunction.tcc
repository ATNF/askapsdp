/// @file
/// @brief Class for a deconvolver based on CLEANing with basis functions.
/// @details This concrete class defines a deconvolver used to estimate an
/// image from a residual image, psf optionally using a weights image.
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
    /// image from a residual image, psf optionally using a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverMultiTermBasisFunction<Double, DComplex>
    /// @ingroup Deconvolver
    
    template<class T, class FT>
    DeconvolverMultiTermBasisFunction<T,FT>::DeconvolverMultiTermBasisFunction(Vector<Array<T> >& dirty,
									       Vector<Array<T> >& psf,
									       Vector<Array<T> >& psfLong)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf), itsDirtyChanged(True), itsBasisFunctionChanged(True),
      itsSolutionType("MAXBASE"), itsDecoupleTerms(true)
    {
      ASKAPLOG_DEBUG_STR(decmtbflogger, "There are " << this->itsNumberTerms << " terms to be solved");

      ASKAPCHECK(psfLong.nelements()==(2*this->itsNumberTerms-1), "Long PSF vector has incorrect length " << psfLong.nelements());
      this->itsPsfLongVec.resize(2*this->itsNumberTerms-1);

      for (uInt term=0;term<(2*this->itsNumberTerms-1);term++) {
	ASKAPCHECK(psfLong(term).nonDegenerate().shape().nelements()==2, "PSF(" << term << ") has too many dimensions " << psfLong(term).shape());
	this->itsPsfLongVec(term)=psfLong(term).nonDegenerate();
      }

    };
    
    template<class T, class FT>
    DeconvolverMultiTermBasisFunction<T,FT>::DeconvolverMultiTermBasisFunction(Array<T>& dirty,
									       Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf), itsDirtyChanged(True), itsBasisFunctionChanged(True),
      itsSolutionType("MAXBASE"), itsDecoupleTerms(true)
    {
      ASKAPLOG_DEBUG_STR(decmtbflogger, "There is only one term to be solved");
      this->itsPsfLongVec.resize(1);
      this->itsPsfLongVec(0)=psf;
    };
    
    template<class T, class FT>
    DeconvolverMultiTermBasisFunction<T,FT>::~DeconvolverMultiTermBasisFunction() {
    };
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::setDecouple(Bool decouple) {
      itsDecoupleTerms=decouple;
    };
    
    template<class T, class FT>
    const Bool DeconvolverMultiTermBasisFunction<T,FT>::decouple() {
      return itsDecoupleTerms;
    };
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::setSolutionType(String sol) {
      itsSolutionType=sol;
    };
    
    template<class T, class FT>
    const String DeconvolverMultiTermBasisFunction<T,FT>::solutionType() {
      return itsSolutionType;
    };
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::setBasisFunction(boost::shared_ptr<BasisFunction<T> > bf) {
      this->itsBasisFunction=bf;
      this->itsBasisFunctionChanged=True;
    };
    
    template<class T, class FT>
    boost::shared_ptr<BasisFunction<T> > DeconvolverMultiTermBasisFunction<T,FT>::basisFunction() {
      return itsBasisFunction;
    };
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::updateDirty(Array<T>& dirty) {
      DeconvolverBase<T,FT>::updateDirty(dirty);
      this->itsDirtyChanged=True;
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::updateDirty(Vector<Array<T> >& dirtyVec) {
      DeconvolverBase<T,FT>::updateDirty(dirtyVec);
      this->itsDirtyChanged=True;
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::configure(const LOFAR::ParameterSet& parset)
    {        
      this->control()->configure(parset);
      
      // Make the basis function
      std::vector<float> defaultScales(3);
      defaultScales[0]=0.0;
      defaultScales[1]=10.0;
      defaultScales[2]=30.0;
      std::vector<float> scales=parset.getFloatVector("scales", defaultScales);
      
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Constructing Multiscale basis function with scales "
			<< scales);
      Bool orthogonal=parset.getBool("orthogonal", "false");
      
      itsBasisFunction = BasisFunction<Float>::ShPtr(new MultiScaleBasisFunction<Float>(scales,
											orthogonal));
      String solutionType=parset.getString("solutiontype", "MAXBASE");
      if(solutionType=="R5") {
	itsSolutionType=solutionType;
      }
      else if(solutionType=="MAXTERM0") {
	itsSolutionType=solutionType;
      }
      else {
	solutionType="MAXBASE";
	itsSolutionType=solutionType;
      }
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Solution type = " << solutionType);
      
      itsDecoupleTerms=parset.getBool("decouple", "true");
      if(itsDecoupleTerms) {
	ASKAPLOG_DEBUG_STR(decmtbflogger, "Decoupling in term using the inverse of the coupling matrix");
      }
    }
      
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::finalise()
    {
      this->updateResiduals(this->itsModel);
      
      for (uInt base=0;base<itsTermBaseFlux.nelements();base++) {
	for (uInt term=0;term<itsTermBaseFlux(base).nelements();term++) {
	  ASKAPLOG_DEBUG_STR(decmtbflogger, "   Term(" <<term << "), Base(" << base
			    << "): Flux = " << itsTermBaseFlux(base)(term));
	}
      }
      
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialiseForBasisFunction(bool force)
    {
      if(!force&&!this->itsBasisFunctionChanged) return;
      
      ASKAPLOG_DEBUG_STR(decmtbflogger,
			"Updating Multi-Term Basis Function deconvolver for change in basis function");

      IPosition subPsfShape(findSubPsfShape());

      // Use a smaller size for the psfs if specified. 
      this->itsBasisFunction->initialise(subPsfShape);
      
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Initialising for PSFs: shape = " << subPsfShape);
      initialisePSF();
      
      itsBasisFunctionChanged=False;
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();
      
      // Initialise residuals
      initialiseResidual();
      
      // Force change in basis function
      initialiseForBasisFunction(true);
      
      this->state()->resetInitialObjectiveFunction();
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialiseResidual()
    {
      
      if(!this->itsDirtyChanged) return;
      
      // Initialise the basis function for residual calculations. 
      this->itsBasisFunction->initialise(this->dirty(0).shape());
      
      ASKAPCHECK(this->itsBasisFunction, "Basis function not initialised");
      
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Shape of basis functions "
			<< this->itsBasisFunction->basisFunction().shape());
       
      uInt nBases(this->itsBasisFunction->numberBases());
      
      itsResidualBasis.resize(nBases);
      for (uInt base=0;base<nBases;base++) {
	itsResidualBasis(base).resize(this->itsNumberTerms);
      }
      
      // Calculate residuals convolved with bases [nx,ny][nterms][nbases]
      ASKAPLOG_DEBUG_STR(decmtbflogger,
			"Calculating convolutions of residual images with basis functions");
      for (uInt base=0;base<nBases;base++) {
	// Calculate transform of residual images [nx,ny,nterms]
	for (uInt term=0;term<this->itsNumberTerms;term++) {
	  
	  // Calculate transform of residual image
	  Array<FT> residualFFT(this->dirty(term).shape().nonDegenerate());
	  residualFFT.set(FT(0.0));
	  casa::setReal(residualFFT, this->dirty(term).nonDegenerate());
	  scimath::fft2d(residualFFT, true);
	  
	  // Calculate transform of basis function [nx,ny,nbases]
	  Matrix<FT> basisFunctionFFT(this->dirty(term).shape().nonDegenerate());
	  basisFunctionFFT.set(FT(0.0));
	  casa::setReal(basisFunctionFFT, Cube<T>(this->itsBasisFunction->basisFunction()).xyPlane(base));
	  scimath::fft2d(basisFunctionFFT, true);
	  
	  // Calculate product and transform back
	  Array<FT> work(this->dirty(term).nonDegenerate().shape());
	  ASKAPASSERT(basisFunctionFFT.shape().conform(residualFFT.shape()));
	  work=conj(basisFunctionFFT)*residualFFT;
	  scimath::fft2d(work, false);
	  
	  // basis function * psf
	  ASKAPLOG_DEBUG_STR(decmtbflogger, "Basis(" << base
			    << ")*Residual(" << term << "): max = " << max(real(work))
			    << " min = " << min(real(work)));
	  
	  this->itsResidualBasis(base)(term)=real(work);
	}
      }
    }
    
    template<class T, class FT>
    IPosition DeconvolverMultiTermBasisFunction<T,FT>::findSubPsfShape() {
      IPosition subPsfShape(2, this->model().shape()(0), this->model().shape()(1));
      // Only use the specified psfWidth if it makes sense
      if(this->control()->psfWidth()>0) {
	uInt psfWidth=this->control()->psfWidth();
	if((psfWidth<this->model().shape()(0))&&(psfWidth<this->model().shape()(1))) {
	  ASKAPLOG_DEBUG_STR(decmtbflogger, "Using subregion of PSF: size " << psfWidth << " pixels");
	  subPsfShape(0)=psfWidth;
	  subPsfShape(1)=psfWidth;
	}
      }
      return subPsfShape;
    }

    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialisePSF()
    {
      
      if(!this->itsBasisFunctionChanged) return;
      
      ASKAPCHECK(this->itsBasisFunction, "Basis function not initialised");
      
      ASKAPLOG_DEBUG_STR(decmtbflogger,
			"Updating Multi-Term Basis Function deconvolver for change in basis function");
      IPosition subPsfShape(findSubPsfShape());

      Array<FT> work(subPsfShape);
      
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Shape of basis functions "
			<< this->itsBasisFunction->basisFunction().shape());
      
      IPosition stackShape(this->itsBasisFunction->basisFunction().shape());
      
      uInt nBases(this->itsBasisFunction->numberBases());
      
      // Now transform the basis functions. These may be a different size from
      // those in initialiseResidual so we don't keep either
      Cube<FT> basisFunctionFFT(this->itsBasisFunction->basisFunction().shape());
      basisFunctionFFT.set(FT(0.0));
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);
      
      itsTermBaseFlux.resize(nBases);
      for(uInt base=0;base<nBases;base++) {
	itsTermBaseFlux(base).resize(this->itsNumberTerms);
	itsTermBaseFlux(base)=0.0;
      }
      
      uInt nx(this->psf(0).shape()(0));
      uInt ny(this->psf(0).shape()(1));
      
      IPosition subPsfStart(2,nx/2-subPsfShape(0)/2,ny/2-subPsfShape(1)/2);
      IPosition subPsfEnd(2,nx/2+subPsfShape(0)/2-1,ny/2+subPsfShape(1)/2-1);
      IPosition subPsfStride(2,1,1);
      
      Slicer subPsfSlicer(subPsfStart, subPsfEnd, subPsfStride, Slicer::endIsLast);
      
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      casa::minMax(minVal, maxVal, minPos, maxPos, this->psf(0).nonDegenerate()(subPsfSlicer));
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Maximum of PSF(0) = " << maxVal << " at " << maxPos);
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Minimum of PSF(0) = " << minVal << " at " << minPos);
      this->itsPeakPSFVal = maxVal;
      this->itsPeakPSFPos(0)=maxPos(0);
      this->itsPeakPSFPos(1)=maxPos(1);
	
      IPosition subPsfPeak(2, this->itsPeakPSFPos(0), this->itsPeakPSFPos(1));
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Peak of PSF subsection at  " << subPsfPeak);
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Shape of PSF subsection is " << subPsfShape);

      // Calculate XFR for the subsection only. We need all PSF's up to
      // 2*nTerms-1
      ASKAPCHECK(this->itsPsfLongVec.nelements()==(2*this->itsNumberTerms-1), "PSF long vector has wrong length " << this->itsPsfLongVec.nelements());

      Vector<Array<FT> > subXFRVec(2*this->itsNumberTerms-1);
      for (uInt term1=0;term1<(2*this->itsNumberTerms-1);term1++) {
	subXFRVec(term1).resize(subPsfShape);
	subXFRVec(term1).set(0.0);
	casa::setReal(subXFRVec(term1), this->itsPsfLongVec(term1).nonDegenerate()(subPsfSlicer));
	scimath::fft2d(subXFRVec(term1), true);
      }

      ASKAPLOG_INFO_STR(decmtbflogger, "About to make cross terms");
      itsPSFCrossTerms.resize(nBases,nBases);
      for (uInt base=0;base<nBases;base++) {
	for (uInt base1=0;base1<nBases;base1++) {
	  itsPSFCrossTerms(base,base1).resize(this->itsNumberTerms, this->itsNumberTerms);
	  for (uInt term1=0;term1<this->itsNumberTerms;term1++) {
	    for (uInt term2=0;term2<this->itsNumberTerms;term2++) {
	      itsPSFCrossTerms(base,base1)(term1,term2).resize(subPsfShape);
	    }
	  }
	}
      }
      
      this->itsCouplingMatrix.resize(nBases);
      for(uInt base1=0;base1<nBases;base1++) {
	itsCouplingMatrix(base1).resize(this->itsNumberTerms,this->itsNumberTerms);
	for (uInt base2=base1;base2<nBases;base2++) {
	  for (uInt term1=0;term1<this->itsNumberTerms;term1++) {
	    for (uInt term2=0;term2<this->itsNumberTerms;term2++) {
	      //	      ASKAPLOG_DEBUG_STR(decmtbflogger, "Calculating convolutions of PSF("
	      //				<< term1 << "+" << term2 << ") with basis functions");
	      work=conj(basisFunctionFFT.xyPlane(base1))*basisFunctionFFT.xyPlane(base2)*
		subXFRVec(term1+term2);
	      scimath::fft2d(work, false);
	      ASKAPLOG_DEBUG_STR(decmtbflogger, "Base(" << base1 << ")*Base(" << base2
				<< ")*PSF(" << term1+term2
				<< "): max = " << max(real(work))
				<< " min = " << min(real(work)));
	      itsPSFCrossTerms(base1,base2)(term1,term2)=real(work);
	      itsPSFCrossTerms(base2,base1)(term1,term2)=itsPSFCrossTerms(base1,base2)(term1,term2);
	      itsPSFCrossTerms(base1,base2)(term2,term1)=itsPSFCrossTerms(base1,base2)(term1,term2);
	      itsPSFCrossTerms(base2,base1)(term2,term1)=itsPSFCrossTerms(base1,base2)(term1,term2);
	      if(base1==base2) {
		itsCouplingMatrix(base1)(term1,term2)=real(work(subPsfPeak));
	      }
	    }
	  }
	}
      }
      
      ASKAPLOG_DEBUG_STR(decmtbflogger, "Calculating inverses of coupling matrices");
      
      // Invert the coupling matrices and check for correctness
      this->itsInverseCouplingMatrix.resize(nBases);
      this->itsDetCouplingMatrix.resize(nBases);
      
      if(this->itsDecoupleTerms) {
	for (uInt base=0;base<nBases;base++) {
	  this->itsInverseCouplingMatrix(base).resize(this->itsNumberTerms,this->itsNumberTerms);
	  ASKAPLOG_DEBUG_STR(decmtbflogger, "Coupling matrix(" << base << ")="
			    << this->itsCouplingMatrix(base));
	  invertSymPosDef(this->itsInverseCouplingMatrix(base),
			  this->itsDetCouplingMatrix(base), this->itsCouplingMatrix(base));
	  ASKAPLOG_DEBUG_STR(decmtbflogger, "Coupling matrix determinant(" << base << ") = "
			    << this->itsDetCouplingMatrix(base));
	  ASKAPLOG_DEBUG_STR(decmtbflogger, "Inverse coupling matrix(" << base
			    << ")=" << this->itsInverseCouplingMatrix(base));
	  // Check that the inverse really is an inverse.
	  Matrix<T> identity(this->itsNumberTerms,this->itsNumberTerms);
	  identity.set(T(0.0));
	  uInt nRows(this->itsCouplingMatrix(base).nrow());
	  uInt nCols(this->itsCouplingMatrix(base).ncolumn());
	  for (uInt row=0;row<nRows;row++) {
	    for (uInt col=0;col<nCols;col++) {
	      identity(row,col)=sum(this->itsCouplingMatrix(base).row(row)
				    * this->itsInverseCouplingMatrix(base).column(col));
	    }
	  }
	  ASKAPLOG_DEBUG_STR(decmtbflogger, "Coupling matrix * inverse " << identity);
	}
      }
      this->itsBasisFunctionChanged=False;
    }
    
    template<class T, class FT>
    bool DeconvolverMultiTermBasisFunction<T,FT>::deconvolve()
    {
      
      this->initialise();
      
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
    void DeconvolverMultiTermBasisFunction<T,FT>::chooseComponent(uInt& optimumBase, casa::IPosition& absPeakPos,
								  Vector<T>& peakValues, Vector<T>& originalValues)
    {
      uInt nBases(this->itsResidualBasis.nelements());
      
      T absPeakVal(0.0);
      
      originalValues.resize(this->itsNumberTerms);


      // Find the base having the peak value in term=0
      // Here the weights image is used as a weight in the determination
      // of the maximum i.e. it finds the max in weight . residual. The values
      // returned are without the weight
      bool isWeighted((this->itsWeight.nelements()>0)&&(this->itsWeight(0).shape().nonDegenerate().conform(this->itsResidualBasis(0)(0).shape())));
      
      Vector<T> minValues(this->itsNumberTerms);
      Vector<T> maxValues(this->itsNumberTerms);

      for(uInt base=0;base<nBases;base++) {
	// Find peak in residual image cube
	casa::IPosition minPos(2,0);
	casa::IPosition maxPos(2,0);
	T minVal(0.0), maxVal(0.0);
	
	// Decouple all terms using inverse coupling matrix
	Vector<Array<T> > coefficients(this->itsNumberTerms);
	for (uInt term1=0;term1<this->itsNumberTerms;term1++) {
	  coefficients(term1).resize(this->dirty(0).shape().nonDegenerate());
	  coefficients(term1).set(T(0.0));
	  for(uInt term2=0;term2<this->itsNumberTerms;term2++) {
	    coefficients(term1)=coefficients(term1)
	      + T(this->itsInverseCouplingMatrix(base)(term1,term2))*this->itsResidualBasis(base)(term2);
	  }
	}

	if(this->itsSolutionType=="R5") {
	  // Now form the criterion image and then search for the peak
	  Array<T> criterion(this->dirty(0).shape().nonDegenerate());
	  criterion.set(T(0.0));
	  for (uInt term1=0;term1<this->itsNumberTerms;term1++) {
	    criterion=criterion+T(2.0)*this->itsResidualBasis(base)(term1)*coefficients(term1);
	    for (uInt term2=0;term2<this->itsNumberTerms;term2++) {
	      criterion=criterion-T(this->itsCouplingMatrix(base)(term1,term2))*coefficients(term1)*coefficients(term2);
	    }
	  }
	  if(isWeighted) {
	    casa::minMaxMasked(minVal, maxVal, minPos, maxPos, criterion,
			       this->itsWeight(0).nonDegenerate());
	  }
	  else {
	    casa::minMax(minVal, maxVal, minPos, maxPos, criterion);
	  }
	  for (uInt term=0;term<this->itsNumberTerms;term++) {
	    minValues(term)=coefficients(term)(minPos);
	    maxValues(term)=coefficients(term)(maxPos);
	  }
	}
	else if(this->itsSolutionType=="MAXTERM0") {
	  if(isWeighted) {
	    casa::minMaxMasked(minVal, maxVal, minPos, maxPos, coefficients(0),
			       this->itsWeight(0).nonDegenerate());
	  }
	  else {
	    casa::minMax(minVal, maxVal, minPos, maxPos, coefficients(0));
	  }
	  for (uInt term=0;term<this->itsNumberTerms;term++) {
	    minValues(term)=coefficients(term)(minPos);
	    maxValues(term)=coefficients(term)(maxPos);
	  }
	}
	else { // MAXBASE
	  if(isWeighted) {
	    casa::minMaxMasked(minVal, maxVal, minPos, maxPos, this->itsResidualBasis(base)(0),
			       this->itsWeight(0).nonDegenerate());
	  }
	  else {
	    casa::minMax(minVal, maxVal, minPos, maxPos, this->itsResidualBasis(base)(0));
	  }
	  for (uInt term=0;term<this->itsNumberTerms;term++) {
	    minValues(term)=this->itsResidualBasis(base)(term)(minPos);
	    maxValues(term)=this->itsResidualBasis(base)(term)(maxPos);
	  }
	  T norm(1/sqrt(this->itsCouplingMatrix(base)(0,0)));
	  maxVal*=norm;
	  minVal*=norm;
	}
      
	if(abs(minVal)>absPeakVal) {
	  optimumBase=base;
	  absPeakVal=abs(minVal);
	  absPeakPos=minPos;
	  peakValues=minValues;
	}
	if(abs(maxVal)>absPeakVal) {
	  optimumBase=base;
	  absPeakVal=abs(maxVal);
	  absPeakPos=maxPos;
	  peakValues=maxValues;
	}
	
	if(isWeighted) {
	  casa::minMaxMasked(minVal, maxVal, minPos, maxPos, this->itsResidualBasis(base)(0),
			     this->itsWeight(0).nonDegenerate());
          originalValues.resize(this->itsNumberTerms);
          for (uInt term=0;term<this->itsNumberTerms;term++) {
            originalValues(term)=this->itsResidualBasis(optimumBase)(term)(absPeakPos)
              *this->itsWeight(0)(absPeakPos);
          }
	}
	else {
	  casa::minMax(minVal, maxVal, minPos, maxPos, this->itsResidualBasis(base)(0));
          originalValues.resize(this->itsNumberTerms);
          for (uInt term=0;term<this->itsNumberTerms;term++) {
            originalValues(term)=this->itsResidualBasis(optimumBase)(term)(absPeakPos);
          }
	}
	// Res: 298.562 Max: 202951 Gain: 0.5 Pos: [493, 397] Scale: 20 Coeffs: 663.175  -701.157   OrigRes: 122.357 -28.997
	//	ASKAPLOG_INFO_STR(decmtbflogger, "Res: " << maxVal << " Max: " << absPeakVal << " Pos: "
	//			  << absPeakPos << " Coeffs: " << peakValues << " OrigRes: " << originalValues);
	
      }
    }
    
    template<class T, class FT>
    bool DeconvolverMultiTermBasisFunction<T,FT>::oneIteration()
    {
      
      // For the psf convolutions, we only need a small part of the
      // basis functions so we recalculate for that size
      IPosition subPsfShape(findSubPsfShape());

      uInt nBases(this->itsResidualBasis.nelements());
      
      casa::IPosition absPeakPos(2,0);
      uInt optimumBase(0);
      Vector<T> peakValues(this->itsNumberTerms);
      Vector<T> originalValues(this->itsNumberTerms);
      chooseComponent(optimumBase, absPeakPos, peakValues, originalValues);
      
      // Report on progress
      // We want the worst case residual
      T absPeakVal=max(abs(originalValues));
      
      //      ASKAPLOG_INFO_STR(decmtbflogger, "All terms: absolute max = " << absPeakVal << " at " << absPeakPos);
      //      ASKAPLOG_INFO_STR(decmtbflogger, "Optimum base = " << optimumBase);
      
      if(this->state()->initialObjectiveFunction()==0.0) {
	this->state()->setInitialObjectiveFunction(abs(absPeakVal));
      }
      this->state()->setPeakResidual(abs(absPeakVal));
      this->state()->setObjectiveFunction(abs(absPeakVal));
      this->state()->setTotalFlux(sum(this->model(0)));
      
      // Now we adjust model and residual for this component
      const casa::IPosition residualShape(this->dirty(0).shape().nonDegenerate());
      const casa::IPosition psfShape(2, this->itsBasisFunction->basisFunction().shape()(0),
				     this->itsBasisFunction->basisFunction().shape()(1));
      
      casa::IPosition residualStart(2,0), residualEnd(2,0), residualStride(2,1);
      casa::IPosition psfStart(2,0), psfEnd(2,0), psfStride(2,1);
      
      const casa::IPosition modelShape(this->model(0).shape().nonDegenerate());
      casa::IPosition modelStart(2,0), modelEnd(2,0), modelStride(2,1);
      
      // Wrangle the start, end, and shape into consistent form. It took me 
      // quite a while to figure this out (slow brain day) so it may be
      // that there are some edge cases for which it fails.
      for (uInt dim=0;dim<2;dim++) {
	residualStart(dim)=max(0, Int(absPeakPos(dim)-psfShape(dim)/2));
	residualEnd(dim)=min(Int(absPeakPos(dim)+psfShape(dim)/2-1), Int(residualShape(dim)-1));
	// Now we have to deal with the PSF. Here we want to use enough of the
	// PSF to clean the residual image.
	psfStart(dim)=max(0, Int(this->itsPeakPSFPos(dim)-(absPeakPos(dim)-residualStart(dim))));
	psfEnd(dim)=min(Int(this->itsPeakPSFPos(dim)-(absPeakPos(dim)-residualEnd(dim))),
			Int(psfShape(dim)-1));
	
	modelStart(dim)=residualStart(dim);
	modelEnd(dim)=residualEnd(dim);
      }

      casa::Slicer psfSlicer(psfStart, psfEnd, psfStride, Slicer::endIsLast);
      casa::Slicer residualSlicer(residualStart, residualEnd, residualStride, Slicer::endIsLast);
      casa::Slicer modelSlicer(modelStart, modelEnd, modelStride, Slicer::endIsLast);
      
      // Add to model
      // We loop over all terms for the optimum base and ignore those terms with no flux
      for (uInt term=0;term<this->itsNumberTerms;term++) {
	if(abs(peakValues(term))>0.0) {
	  this->model(term).nonDegenerate()(modelSlicer)
	    = this->model(term).nonDegenerate()(modelSlicer)
	    + this->control()->gain()*peakValues(term)*
	    Cube<T>(this->itsBasisFunction->basisFunction()).xyPlane(optimumBase).nonDegenerate()(psfSlicer);
	  this->itsTermBaseFlux(optimumBase)(term)+=this->control()->gain()*peakValues(term);
	}
      }	
      
      // Subtract PSFs, including base-base crossterms
      for (uInt term1=0;term1<this->itsNumberTerms;term1++) {
	for (uInt term2=0;term2<this->itsNumberTerms;term2++) {
	  if(abs(peakValues(term2))>0.0) {
	    for (uInt base=0;base<nBases;base++) {
	      this->itsResidualBasis(base)(term1)(residualSlicer) =
		this->itsResidualBasis(base)(term1)(residualSlicer)
		- this->control()->gain()*peakValues(term2) *
		this->itsPSFCrossTerms(base,optimumBase)(term1,term2)(psfSlicer);

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
