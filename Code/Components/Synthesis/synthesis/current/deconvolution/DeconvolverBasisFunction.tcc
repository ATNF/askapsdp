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

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <string>

#include <deconvolution/DeconvolverBasisFunction.h>

namespace askap {

  namespace synthesis {

    /// @brief Class for a deconvolver based on the BasisFunction Clean
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a residual image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverBasisFunction<Double, DComplex>
    /// @ingroup Deconvolver

    template<class T, class FT>
    DeconvolverBasisFunction<T,FT>::DeconvolverBasisFunction(Array<T>& dirty, Array<T>& psf,
                                                             Bool useCrossTerms)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf), itsUseCrossTerms(useCrossTerms)
    {
      this->model().resize(this->dirty().shape());
      this->model().set(T(0.0));
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
    void DeconvolverBasisFunction<T,FT>::finalise()
    {
      Array<FT> work;
      // Find residuals for current model model
      work.resize(this->model().shape());
      work.set(FT(0.0));
      casa::setReal(work, this->model());
      scimath::fft2d(work, true);
      work=this->xfr()*work;
      scimath::fft2d(work, false);
      this->residual()=this->dirty()-real(work);
    }
      
    template<class T, class FT>
    void DeconvolverBasisFunction<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();

      // Now calculate the convolutions of the residual images and PSFs 
      // with the basis functions
      IPosition stackShape(3, this->psf().shape()[0], this->psf().shape()[1],
                           this->itsBasisFunction->numberTerms());

      Array<FT> residualFT(this->residual().shape());
      residualFT.set(FT(0.0));
      casa::setReal(residualFT, this->residual());
      scimath::fft2d(residualFT, false);

      itsResidualBasisFunction.resize(stackShape);
      itsPSFBasisFunction.resize(stackShape);

      Array<FT> bfPlane(this->model().shape());

      Cube<FT> basisFunctionFFT(stackShape);
      casa::setReal(basisFunctionFFT, this->itsBasisFunction->basisFunction());
      scimath::fft2d(basisFunctionFFT, true);

      Array<FT> residualFFT(this->itsResidual.shape());
      casa::setReal(residualFFT, this->itsResidual);
      scimath::fft2d(residualFFT, true);

      Array<FT> work(this->model().shape());
      ASKAPLOG_INFO_STR(logger, "Calculating convolutions of residual image with basis functions");

      for (uInt term=0;term<this->itsBasisFunction->numberTerms();term++) {

        work=conj(basisFunctionFFT.xyPlane(term).nonDegenerate())*residualFFT;
        scimath::fft2d(work, false);

        // basis function * residual
        ASKAPLOG_INFO_STR(logger, "Basis function(" << term << ") * Residual: max = " << max(real(work)) << " min = " << min(real(work)));

        Cube<T>(itsResidualBasisFunction).xyPlane(term)=real(work);
      }
      ASKAPLOG_INFO_STR(logger, "Calculating convolutions of PSFs with basis functions");
      for (uInt term=0;term<this->itsBasisFunction->numberTerms();term++) {
          // basis function * psf
          work=conj(basisFunctionFFT.xyPlane(term).nonDegenerate())*this->xfr();
          scimath::fft2d(work, false);
          Cube<T>(itsPSFBasisFunction).xyPlane(term)=real(work);
          
          ASKAPLOG_INFO_STR(logger, "Basis function(" << term << ") * PSF: max = " << max(real(work)) << " min = " << min(real(work)));
      }

      if(this->itsUseCrossTerms) {
        ASKAPLOG_INFO_STR(logger, "Calculating double convolutions of PSF with basis functions");
        // Find peak in residual image cube
        casa::IPosition minPos;
        casa::IPosition maxPos;
        T minVal, maxVal;

        IPosition crossTermsShape(4, this->psf().shape()[0], this->psf().shape()[1],
                                  this->itsBasisFunction->numberTerms(),
                                  this->itsBasisFunction->numberTerms());
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
              conj(basisFunctionFFT.xyPlane(term1)).nonDegenerate()*basisFunctionFFT.xyPlane(term).nonDegenerate()*this->xfr();
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
            ASKAPLOG_INFO_STR(logger, "Basis function(" << term << ") * Basis function(" << term1
                              << ") * PSF: max = " << maxVal << " min = " << minVal);
            this->itsCouplingMatrix(term, term1) = maxVal;
          }
        }
        ASKAPLOG_INFO_STR(logger, "Coupling matrix " << this->itsCouplingMatrix);
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

    // This contains the heart of the BasisFunction Clean algorithm
    // The residual image and psfs are intrinsically two dimensional
    // but are expanded by projection onto the basis functions
    template<class T, class FT>
    bool DeconvolverBasisFunction<T,FT>::oneIteration()
    {
      bool isMasked(this->itsWeightedMask.shape().conform(this->itsResidualBasisFunction.shape()));

      // Find peak in residual image cube
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      if (isMasked) {
        casa::minMaxMasked(minVal, maxVal, minPos, maxPos, this->itsResidualBasisFunction, this->itsWeightedMask);
      }
      else {
        casa::minMax(minVal, maxVal, minPos, maxPos, this->itsResidualBasisFunction);
      }
      //
      ASKAPLOG_INFO_STR(logger, "Maximum =  " << maxVal << " at location " << maxPos);
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

      casa::IPosition residualShape(this->itsResidualBasisFunction.shape());
      casa::IPosition psfShape(this->itsPSFBasisFunction.shape());
      casa::uInt ndim(this->itsResidualBasisFunction.shape().size());

      casa::IPosition residualStart(ndim,0), residualEnd(ndim,0), residualStride(ndim,1);
      casa::IPosition psfStart(ndim,0), psfEnd(ndim,0), psfStride(ndim,1);
      casa::IPosition modelStart(ndim-1,0), modelEnd(ndim-1,0), modelStride(ndim-1,1);
      casa::IPosition psfCrossTermsStart(ndim+1,0), psfCrossTermsEnd(ndim+1,0), psfCrossTermsStride(ndim+1,1);

      Int psfWidth=this->itsPSFBasisFunction.shape()(0);
      psfWidth=(psfWidth-psfWidth%2)/2;

      // Only use the specified psfWidth if it makes sense
      if((this->control()->psfWidth()>0)&&(this->control()->psfWidth()<psfWidth)) {
        psfWidth=(this->control()->psfWidth()-this->control()->psfWidth()%2);
      }
      for (uInt dim=0;dim<2;dim++) {
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
        psfCrossTermsStart(dim)=psfStart(dim);
        psfCrossTermsEnd(dim)=psfEnd(dim);
        modelStart(dim)=residualStart(dim);
        modelEnd(dim)=residualEnd(dim);
      }

      // For the model, we just have to update using the optimum plane
      uInt optimumPlane(absPeakPos(2));
      psfStart(2)=psfEnd(2)=optimumPlane;
      residualStart(2)=residualEnd(2)=optimumPlane;
      casa::Slicer psfSlicer(psfStart, psfEnd, psfStride, Slicer::endIsLast);

      //      if(!(residualSlicer.length()==psfSlicer.length())||!(residualSlicer.stride()==psfSlicer.stride())) {
      //        ASKAPLOG_INFO_STR(logger, "Peak of PSF  : " << this->itsPeakPSFPos );
      //        ASKAPLOG_INFO_STR(logger, "Peak of residual: " << absPeakPos );
      //        ASKAPLOG_INFO_STR(logger, "PSF width    : " << psfWidth );
      //        ASKAPLOG_INFO_STR(logger, "Residual start  : " << residualStart << " end: " << residualEnd );
      //        ASKAPLOG_INFO_STR(logger, "PSF   start  : " << psfStart << " end: " << psfEnd );
      //        ASKAPLOG_INFO_STR(logger, "Model shape  : " << this->model().shape());
      //        ASKAPLOG_INFO_STR(logger, "Basis fn     : " << this->itsBasisFunction->basisFunction().shape());
      //        throw AskapError("Mismatch in slicers for residual and psf images");
      //      }

      // Add to model
      // Note that the model is only two dimensional. If desired, we could make it three dimensional
      // and keep the model layers separate
      {
        casa::Slicer modelSlicer(modelStart, modelEnd, modelStride, Slicer::endIsLast);
        this->model()(modelSlicer) = this->model()(modelSlicer)
          + this->control()->gain()*absPeakVal*this->itsBasisFunction->basisFunction()(psfSlicer).nonDegenerate();
      }
      
      // Subtract PSF for this plane from residual image for the same plane
      {
        casa::Slicer residualSlicer(residualStart, residualEnd, residualStride, Slicer::endIsLast);
        this->itsResidualBasisFunction(residualSlicer) = this->itsResidualBasisFunction(residualSlicer)
          - this->control()->gain()*absPeakVal*this->itsPSFBasisFunction(psfSlicer);
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

  } // namespace synthesis

} // namespace askap


