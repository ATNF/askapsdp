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
    DeconvolverMultiTermBasisFunction<T,FT>::DeconvolverMultiTermBasisFunction(Vector<Array<T> >& dirty,
									       Vector<Array<T> >& psf,
									       Vector<Array<T> >& psfLong)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf), itsDirtyChanged(True), itsBasisFunctionChanged(True),
      itsSolutionType(MAXBASE)
    {
      ASKAPLOG_INFO_STR(decmtbflogger, "There are " << this->itsNumberTerms << " terms to be solved");
      this->itsPsfLongVec=psfLong;
    };
    
    template<class T, class FT>
    DeconvolverMultiTermBasisFunction<T,FT>::DeconvolverMultiTermBasisFunction(Array<T>& dirty,
									       Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf), itsDirtyChanged(True), itsBasisFunctionChanged(True),
      itsSolutionType(MAXBASE)
    {
      ASKAPLOG_INFO_STR(decmtbflogger, "There are " << this->itsNumberTerms << " terms to be solved");
      this->itsPsfLongVec.resize(1);
      this->itsPsfLongVec(0)=psf;
    };
    
    template<class T, class FT>
    DeconvolverMultiTermBasisFunction<T,FT>::~DeconvolverMultiTermBasisFunction() {
    };
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::setSolutionType(SOLUTION sol) {
      itsSolutionType=sol;
    };

    template<class T, class FT>
    const uInt DeconvolverMultiTermBasisFunction<T,FT>::solutionType() {
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
      if (!dirty.shape().conform(this->dirty(0).shape())) {
        throw(AskapError("Updated dirty image has different shape"));
      }
      this->itsDirty.resize(1);
      this->itsDirty(0)=dirty;
      this->itsDirtyChanged=True;
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::updateDirty(Vector<Array<T> >& dirtyVec) {
      if (dirtyVec.nelements()!=this->itsDirty.nelements()) {
        throw(AskapError("Updated dirty image has different shape"));
      }
      this->itsDirty=dirtyVec;
      this->itsDirtyChanged=True;
    }
    
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
	String solutionType=parset.getString("solutiontype", "MAXBASE");
	if(solutionType=="R5") {
	  itsSolutionType=R5;
	}
	else if(solutionType=="MAXTERM0") {
	  itsSolutionType=MAXTERM0;
	}
	else {
	  itsSolutionType=MAXBASE;
	  solutionType="MAXBASE";
	}
	ASKAPLOG_INFO_STR(decmtbflogger, "Solution type = " << solutionType);
      }
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::finalise()
    {
      this->updateResiduals(this->itsModel);
      
      for (uInt base=0;base<itsTermBaseFlux.nelements();base++) {
	for (uInt term=0;term<itsTermBaseFlux(base).nelements();term++) {
	  ASKAPLOG_INFO_STR(decmtbflogger, "   Term(" <<term << "), Base(" << base
			    << "): Flux = " << itsTermBaseFlux(base)(term));
	}
      }
      
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialiseForBasisFunction()
    {
      if(!this->itsBasisFunctionChanged) return;

      ASKAPLOG_INFO_STR(decmtbflogger,
			"Updating Multi-Term Basis Function deconvolver for change in basis function");
      Int psfWidth=this->model().shape()(0);
      // Only use the specified psfWidth if it makes sense
      if((this->control()->psfWidth()>0)&&(this->control()->psfWidth()<psfWidth)) {
	psfWidth=this->control()->psfWidth();
	ASKAPLOG_INFO_STR(decmtbflogger, "Using subregion of PSF: size " << psfWidth
			  << " pixels");
      }
      IPosition subPsfShape(2, psfWidth, psfWidth);
      // Use a smaller size for the psfs if specified. 
      this->itsBasisFunction->initialise(subPsfShape);

      ASKAPLOG_INFO_STR(decmtbflogger, "Initialising for PSFs: shape = " << subPsfShape);
      initialisePSF();

      // Calculate the term coupling matrix
      ASKAPLOG_INFO_STR(decmtbflogger, "Calculating term coupling matrix");
      calculateTermCoupling();
      
      itsBasisFunctionChanged=False;
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();

      // Initialise residuals
      initialiseResidual();

      // Initialise for change in basis function
      initialiseForBasisFunction();

      this->state()->resetInitialObjectiveFunction();
      
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialiseResidual()
    {

      if(!this->itsDirtyChanged) return;

      // Initialise the basis function for residual calculations. 
      this->itsBasisFunction->initialise(this->dirty(0).shape());

      ASKAPCHECK(this->itsBasisFunction, "Basis function not initialised");
      
      ASKAPLOG_INFO_STR(decmtbflogger, "Shape of basis functions "
			<< this->itsBasisFunction->basisFunction().shape());
      
      uInt nBases(this->itsBasisFunction->numberBases());

      itsResidualBasis.resize(nBases);
      for (uInt base=0;base<nBases;base++) {
	itsResidualBasis(base).resize(this->itsNumberTerms);
      }
      
      // Calculate transform of basis function [nx,ny,nbases]
      Cube<FT> basisFunctionFFT(this->itsBasisFunction->basisFunction().shape());
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);
      
      // Calculate residuals convolved with bases [nx,ny][nterms][nbases]
      ASKAPLOG_INFO_STR(decmtbflogger,
			"Calculating convolutions of residual images with basis functions");
      for (uInt base=0;base<nBases;base++) {
	// Calculate transform of residual images [nx,ny,nterms]
	for (uInt term=0;term<this->itsNumberTerms;term++) {
	  Array<FT> residualFFT(this->dirty(term).shape().nonDegenerate());
	  residualFFT.set(FT(0.0));
	  casa::setReal(residualFFT, this->dirty(term).nonDegenerate());
	  scimath::fft2d(residualFFT, true);
      
	  Array<FT> work(this->dirty(term).nonDegenerate().shape());
	  ASKAPASSERT(basisFunctionFFT.xyPlane(base).shape().conform(residualFFT.shape()));
	  work=conj(basisFunctionFFT.xyPlane(base))*residualFFT;
	  scimath::fft2d(work, false);
	
	  // basis function * psf
	  ASKAPLOG_INFO_STR(decmtbflogger, "Basis(" << base
			    << ")*PSF(" << term << "): max = " << max(real(work))
			    << " min = " << min(real(work)));
	
	  this->itsResidualBasis(base)(term)=real(work);
	}
      }
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::initialisePSF()
    {

      if(!this->itsBasisFunctionChanged) return;

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

      // Now transform the basis functions. These may be a different size from
      // those in initialiseResidual so we don't keep either
      Cube<FT> basisFunctionFFT(this->itsBasisFunction->basisFunction().shape());
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);

      itsTermBaseFlux.resize(nBases);
      for(uInt base=0;base<nBases;base++) {
	itsTermBaseFlux(base).resize(this->itsNumberTerms);
	itsTermBaseFlux(base)=0.0;
      }

      // Calculate XFR for the subsection only
      Array<FT> subXFR(subPsfShape);
       
      itsPSFBasis.resize(nBases);
      for (uInt base=0;base<nBases;base++) {
	itsPSFBasis(base).resize(this->itsNumberTerms);
	for (uInt term=0;term<this->itsNumberTerms;term++) {
	  itsPSFBasis(base)(term).resize(subPsfShape);
	}
      }
      itsPSFBasisBasis.resize(nBases,nBases);

      for (uInt base=0;base<nBases;base++) {
	for (uInt base1=base;base1<nBases;base1++) {
	  itsPSFBasisBasis(base,base1).resize(this->itsNumberTerms);
	  for (uInt term=0;term<this->itsNumberTerms;term++) {
	    itsPSFBasisBasis(base,base1)(term).resize(subPsfShape);
	  }
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
	ASKAPLOG_INFO_STR(logger, "Maximum of PSF(" << term << ") = " << maxVal << " at " << maxPos);
	ASKAPLOG_INFO_STR(logger, "Minimum of PSF(" << term << ") = " << minVal << " at " << minPos);
	this->itsPeakPSFVal(term) = maxVal;
	this->itsPeakPSFPos(term)(0)=maxPos(0);
	this->itsPeakPSFPos(term)(1)=maxPos(1);
	
	casa::setReal(subXFR, this->psf(term).nonDegenerate()(subPsfSlicer));
	scimath::fft2d(subXFR, true);
      
	// Now we have all the ingredients to calculate the convolutions
	// of basis function with psf's, etc. for bach basis
	ASKAPLOG_INFO_STR(decmtbflogger, "Calculating convolutions of PSF(" << term << ") with basis functions");
	for(uInt base=0;base<nBases;base++) {
	  ASKAPASSERT(basisFunctionFFT.xyPlane(base).nonDegenerate().shape().conform(subXFR.shape()));
	  work=conj(basisFunctionFFT.xyPlane(base).nonDegenerate())*subXFR;
	  scimath::fft2d(work, false);
	  ASKAPLOG_INFO_STR(decmtbflogger, "Base(" << base << ")*PSF(" << term << "): max = "
			    << max(real(work)) << " min = " << min(real(work)));
	  itsPSFBasis(base)(term)=real(work);

	  // Now do the cross terms
	  for (uInt base1=base;base1<nBases;base1++) {
	    work=conj(basisFunctionFFT.xyPlane(base))*basisFunctionFFT.xyPlane(base1)*subXFR;
	    scimath::fft2d(work, false);
	    ASKAPLOG_INFO_STR(decmtbflogger, "Base(" << base << ")*Base(" << base1
			      << ")*PSF(" << term
			      << "): max = " << max(real(work))
			      << " min = " << min(real(work)));

	    itsPSFBasisBasis(base,base1)(term)=real(work);
	  }
	}
      }
      this->itsBasisFunctionChanged=False;
    }
    
    template<class T, class FT>
    void DeconvolverMultiTermBasisFunction<T,FT>::calculateTermCoupling()
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
      ASKAPLOG_INFO_STR(decmtbflogger, "Calculating coupling matrix from "
			<< this->itsPsfLongVec.nelements() << " PSFs");

      Vector<Array<FT> > subXFRVec(this->itsPsfLongVec.nelements());
    
      // Transform all the PSFs: note that we need all the PSF terms up to
      // 2*N-1 so that we can use the prescription from Urvashi:
      // B(i)*B(j)=B(0)*B(i+j)
      // We need all of these upfront so we can do the cross terms
      for (uInt term=0;term<(2*nTerms-1);term++) {
	subXFRVec(term).resize(subPsfShape);
	casa::setReal(subXFRVec(term), this->itsPsfLongVec(term).nonDegenerate()(subPsfSlicer));
	scimath::fft2d(subXFRVec(term), true);
      }
      
      // Now we can calculate the peak of the cross terms. These are all we need
      // to get the term coupling matrix.
      ASKAPLOG_INFO_STR(decmtbflogger,
			"Calculating convolutions of Psfs with basis functions, and coupling matrices");
      this->itsCouplingMatrix.resize(nBases);
      ASKAPASSERT(basisFunctionFFT.xyPlane(0).nonDegenerate().shape().conform(subXFRVec(0).shape()));
      for (uInt base=0;base<nBases;base++) {
	this->itsCouplingMatrix(base).resize(nTerms,nTerms);
	Array<FT> bf2(basisFunctionFFT.xyPlane(base)*conj(basisFunctionFFT.xyPlane(base)));
	for (uInt term1=0;term1<nTerms;term1++) {
	  for (uInt term2=0;term2<nTerms;term2++) {
	    work=bf2*subXFRVec(term1+term2)*conj(subXFRVec(0));
	    scimath::fft2d(work, false);
	    itsCouplingMatrix(base)(term1,term2)=real(work(this->itsPeakPSFPos(0)));
	  }
	}
      }

      ASKAPLOG_INFO_STR(decmtbflogger, "Calculating inverses of coupling matrices");

      // Invert the coupling matrices and check for correctness
      this->itsInverseCouplingMatrix.resize(nBases);
      this->itsDetCouplingMatrix.resize(nBases);

      for (uInt base=0;base<nBases;base++) {
	this->itsInverseCouplingMatrix(base).resize(nTerms,nTerms);
	ASKAPLOG_INFO_STR(decmtbflogger, "Coupling matrix(" << base << ")=" << this->itsCouplingMatrix(base));
	invertSymPosDef(this->itsInverseCouplingMatrix(base), this->itsDetCouplingMatrix(base), this->itsCouplingMatrix(base));
	ASKAPLOG_INFO_STR(decmtbflogger, "Coupling matrix determinant(" << base << ") = " << this->itsDetCouplingMatrix(base));
	//	ASKAPLOG_INFO_STR(decmtbflogger, "Inverse coupling matrix(" << base << ")=" << this->itsInverseCouplingMatrix(base));
	// Check that the inverse really is an inverse.
	Matrix<T> identity(nTerms,nTerms);
	identity.set(T(0.0));
	uInt nRows(this->itsCouplingMatrix(base).nrow());
	uInt nCols(this->itsCouplingMatrix(base).ncolumn());
	for (uInt row=0;row<nRows;row++) {
	  for (uInt col=0;col<nCols;col++) {
	    identity(row,col)=sum(this->itsCouplingMatrix(base).row(row)*this->itsInverseCouplingMatrix(base).column(col));
	  }
	}
	//	ASKAPLOG_INFO_STR(decmtbflogger, "Coupling matrix * inverse " << identity);
      }
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
    void DeconvolverMultiTermBasisFunction<T,FT>::chooseComponent(uInt& optimumBase, casa::IPosition& absPeakPos)
    {
      Bool verbose(False);

      uInt nBases(this->itsResidualBasis.nelements());
      uInt nTerms(this->itsNumberTerms);

      T absPeakVal(0.0);

      // Find the base having the peak value in term=0
      // Here the weighted mask is used as a weight in the determination
      // of the maximum i.e. it finds the max in mask . residual. The values
      // returned are without the mask
      bool isMasked((this->itsMask.nelements()>0)&&(this->itsMask(0).shape().nonDegenerate().conform(this->itsResidualBasis(0)(0).shape())));

      for(uInt base=0;base<nBases;base++) {
	// Find peak in residual image cube
	casa::IPosition minPos(2,0);
	casa::IPosition maxPos(2,0);
	T minVal(0.0), maxVal(0.0);

	switch (this->itsSolutionType) {
	case R5:
	  {
	    // Decouple all terms using inverse coupling matrix
	    Vector<Array<T> > coefficients(nTerms);
	    for (uInt term1=0;term1<nTerms;term1++) {
	      coefficients(term1).resize(this->dirty(0).shape().nonDegenerate());
	      coefficients(term1).set(T(0.0));
	      for(uInt term2=0;term2<nTerms;term2++) {
		coefficients(term1)=coefficients(term1)
		  + T(this->itsInverseCouplingMatrix(base)(term1,term2))*this->itsResidualBasis(base)(term2);
	      }
	    }
	    // Now form the criterion for a peak
	    Array<T> criterion(this->dirty(0).shape().nonDegenerate());
	    criterion.set(T(0.0));
	    for (uInt term1=0;term1<nTerms;term1++) {
	      criterion=criterion+T(2.0)*this->itsResidualBasis(base)(term1)*coefficients(term1);
	      for (uInt term2=0;term2<nTerms;term2++) {
		criterion=criterion-T(this->itsCouplingMatrix(base)(term1,term2))*coefficients(term1)*coefficients(term2);
	      }
	    }
	    if(isMasked) {
	      casa::minMaxMasked(minVal, maxVal, minPos, maxPos, criterion, this->itsMask(0).nonDegenerate());
	    }
	    else {
	      casa::minMax(minVal, maxVal, minPos, maxPos, criterion);
	    }
	  }
	case MAXTERM0:
	  {
	    Array<T> coefficients;
	    coefficients.resize(this->dirty(0).shape().nonDegenerate());
	    coefficients.set(T(0.0));
	    for(uInt term=0;term<nTerms;term++) {
	      coefficients=coefficients
		+ T(this->itsInverseCouplingMatrix(base)(0,term))*this->itsResidualBasis(base)(term);
	    }
	    if(isMasked) {
	      casa::minMaxMasked(minVal, maxVal, minPos, maxPos, coefficients,
				 this->itsMask(0).nonDegenerate());
	    }
	    else {
	      casa::minMax(minVal, maxVal, minPos, maxPos, coefficients);
	    }
	  }
	case MAXBASE:
	default:
	  {
	    if(isMasked) {
	      casa::minMaxMasked(minVal, maxVal, minPos, maxPos, this->itsResidualBasis(base)(0),
				 this->itsMask(0).nonDegenerate());
	    }
	    else {
	      casa::minMax(minVal, maxVal, minPos, maxPos, this->itsResidualBasis(base)(0));
	    }
	    T norm(1/sqrt(this->itsCouplingMatrix(base)(0,0)));
	    maxVal*=norm;
	    minVal*=norm;
	  }
	}

	if(verbose) ASKAPLOG_INFO_STR(decmtbflogger, "Base " << base << ": min, max " << minVal << " " << maxVal
			  << " Peak: max, pos, base " << absPeakVal << " " << absPeakPos << optimumBase);

	if(abs(minVal)>absPeakVal) {
	  optimumBase=base;
	  absPeakVal=abs(minVal);
	  absPeakPos=minPos;
	}
	if(abs(maxVal)>absPeakVal) {
	  optimumBase=base;
	  absPeakVal=abs(maxVal);
	  absPeakPos=maxPos;
	}
      }
    }
    
    template<class T, class FT>
    bool DeconvolverMultiTermBasisFunction<T,FT>::oneIteration()
    {

      Bool verbose(False);

      uInt nBases(this->itsResidualBasis.nelements());
      uInt nTerms(this->itsNumberTerms);

      casa::IPosition absPeakPos(2,0);
      uInt optimumBase(0);
      chooseComponent(optimumBase, absPeakPos);

      // Find the vector of values for the optimum base
      Vector<T> peakValues(nTerms);
      for (uInt term=0;term<nTerms;term++) {
	peakValues(term)=itsResidualBasis(optimumBase)(term)(absPeakPos);
      }
      // Report on progress
      // We want the worst case residual
      T absPeakVal=abs(itsResidualBasis(0)(0)(absPeakPos));

      if(verbose) ASKAPLOG_INFO_STR(decmtbflogger, "All terms: abs max = " << absPeakVal << " at " << absPeakPos);
      if(verbose) ASKAPLOG_INFO_STR(decmtbflogger, "Optimum base = " << optimumBase);

      if(this->state()->initialObjectiveFunction()==0.0) {
	this->state()->setInitialObjectiveFunction(abs(absPeakVal));
      }
      this->state()->setPeakResidual(abs(absPeakVal));
      this->state()->setObjectiveFunction(abs(absPeakVal));
      this->state()->setTotalFlux(sum(this->model(0)));
      
      // Now we adjust model and residual for this component
      const casa::IPosition residualShape(this->dirty(0).shape().nonDegenerate());
      const casa::IPosition psfShape(this->psf(0).shape().nonDegenerate());
      
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
	  this->model(term).nonDegenerate()(modelSlicer) = this->model(term).nonDegenerate()(modelSlicer)
	    + this->control()->gain()*peakValues(term)*
	    Cube<T>(this->itsBasisFunction->basisFunction()).xyPlane(optimumBase).nonDegenerate()(psfSlicer);
	  this->itsTermBaseFlux(optimumBase)(term)+=this->control()->gain()*peakValues(term);
	}
      }	
      
      // Subtract PSFs, including base-base crossterms
      for (uInt term=0;term<nTerms;term++) {
	if(abs(peakValues(term))>0.0) {
	  this->itsResidualBasis(optimumBase)(term)(residualSlicer) =
	    this->itsResidualBasis(optimumBase)(term)(residualSlicer)
	    - this->control()->gain()*peakValues(term)*this->itsPSFBasis(optimumBase)(term)(psfSlicer);

	  // Now do cross terms
	  for (uInt base=0;base<nBases;base++) {
	    if(base<optimumBase) {
	      this->itsResidualBasis(base)(term)(residualSlicer) =
		this->itsResidualBasis(base)(term)(residualSlicer)
		- this->control()->gain()*peakValues(term) *
		this->itsPSFBasisBasis(base,optimumBase)(term)(psfSlicer);
	    }
	    if(base>optimumBase) {
	      this->itsResidualBasis(base)(term)(residualSlicer) =
		this->itsResidualBasis(base)(term)(residualSlicer)
		- this->control()->gain()*peakValues(term) *
		this->itsPSFBasisBasis(optimumBase,base)(term)(psfSlicer);
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
