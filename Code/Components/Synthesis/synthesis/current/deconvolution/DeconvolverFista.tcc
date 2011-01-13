/// @file
/// @brief Class for a deconvolver based on the Fista
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
ASKAP_LOGGER(decfistalogger, ".deconvolution.fista");

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <fft/FFTWrapper.h>

#include <string>

#include <deconvolution/DeconvolverFista.h>

#include <deconvolution/MultiScaleBasisFunction.h>

#include <measurementequation/SynthesisParamsHelper.h>

namespace askap {

  namespace synthesis {

    /// @brief Class for a deconvolver based on the Fista Clean
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverFista<Double, DComplex>
    /// @ingroup Deconvolver

    template<class T, class FT>
    DeconvolverFista<T,FT>::~DeconvolverFista() {
    };

    template<class T, class FT>
    DeconvolverFista<T,FT>::DeconvolverFista(Vector<Array<T> >& dirty, Vector<Array<T> >& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf)
    {
    };
    
    template<class T, class FT>
    DeconvolverFista<T,FT>::DeconvolverFista(Array<T>& dirty, Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf)
    {
    };
    
    template<class T, class FT>
    void DeconvolverFista<T,FT>::configure(const LOFAR::ParameterSet& parset)
    {        
      DeconvolverBase<T,FT>::configure(parset);
      
    }

    template<class T, class FT>
    void DeconvolverFista<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();

      // Initialise the residual image
      this->residual().resize(this->dirty().shape());
      this->residual()=this->dirty().copy();
 
      if(itsBasisFunction) {
	this->itsBasisFunction->initialise(this->model().shape());
	itsBasisFunctionTransform.resize(itsBasisFunction->basisFunction().shape());
	casa::setReal(itsBasisFunctionTransform, itsBasisFunction->basisFunction().nonDegenerate());
	scimath::fft2d(itsBasisFunctionTransform, true);
      }

      ASKAPLOG_INFO_STR(decfistalogger, "Initialised FISTA solver");
   }

    template<class T, class FT>
    bool DeconvolverFista<T,FT>::deconvolve()
    {

      this->initialise();

      bool isMasked(this->itsWeightedMask(0).shape().conform(this->dirty().shape()));

      Array<T> X, X_old, X_temp;

      X_temp.resize(this->model().shape());
      X_temp.set(T(0.0));

      X_old.resize(this->model().shape());
      X_old.set(T(0.0));

      X.resize(this->model().shape());
      X=this->background().copy();

      T absPeakVal;

      ASKAPLOG_INFO_STR(decfistalogger, "Performing Fista for " << this->control()->targetIter() << " iterations");

      updateResiduals(X);

      X_temp=X.copy();

      absPeakVal=max(abs(this->residual()));

      T effectiveLambda(absPeakVal*this->control()->fractionalThreshold()+this->control()->lambda());
      ASKAPLOG_INFO_STR(decfistalogger, "Effective lambda = " << effectiveLambda);
      
      T t_new=1;

      do {
	X_old=X_temp.copy();
	T t_old=t_new;

	updateResiduals(X);
	SynthesisParamsHelper::saveAsCasaImage("residuals.tab", this->residual());

	X=X+this->residual()/this->itsLipschitz(0);

	// Transform to other (e.g. multiscale) space
	Array<T> WX;
	SynthesisParamsHelper::saveAsCasaImage("X.tab", X);
	this->W(WX,X);
	SynthesisParamsHelper::saveAsCasaImage("W.tab", WX);

	// Now shrink the coefficients towards zero and clip those below
	// lambda/lipschitz.
	Array<T> shrink(WX.shape());

	Array<T> truncated(abs(WX)-effectiveLambda/this->itsLipschitz(0));
	shrink=truncated(truncated>T(0.0));
	shrink=sign(WX)*shrink;
	shrink(truncated<T(0.0))=T(0.0);

	//	SynthesisParamsHelper::saveAsCasaImage("shrink.tab", WX);
	// Transform back from other (e.g. wavelet) space here
	
	this->WT(X_temp,shrink);
	SynthesisParamsHelper::saveAsCasaImage("WT.tab", X_temp);

	t_new=(T(1.0)+sqrt(T(1.0)+T(4.0)*square(t_old)))/T(2.0);
	X=X_temp+((t_old-T(1.0))/t_new)*(X_temp-X_old);
        {
	  casa::IPosition minPos;
          casa::IPosition maxPos;
          T minVal(0.0), maxVal(0.0);
          if (isMasked) {
            casa::minMaxMasked(minVal, maxVal, minPos, maxPos,
			       this->residual(),
                               this->itsWeightedMask(0));
          }
          else {
            casa::minMax(minVal, maxVal, minPos, maxPos, this->residual());
          }
          //
          ASKAPLOG_INFO_STR(decfistalogger, "   Maximum = " << maxVal << " at location " << maxPos);
          ASKAPLOG_INFO_STR(decfistalogger, "   Minimum = " << minVal << " at location " << minPos);
          if(abs(minVal)<abs(maxVal)) {
            absPeakVal=abs(maxVal);
          }
          else {
            absPeakVal=abs(minVal);
          }
        }

	T l1Norm=sum(abs(X_temp));
	T fit=casa::sum(this->residual()*this->residual());
	T objectiveFunction(fit+effectiveLambda*l1Norm);
        this->state()->setPeakResidual(absPeakVal);
        this->state()->setObjectiveFunction(objectiveFunction);
        this->state()->setTotalFlux(sum(X_temp));
        
        this->monitor()->monitor(*(this->state()));
        this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));
      this->model()=X_temp.copy();
      updateResiduals(this->model());
      
      ASKAPLOG_INFO_STR(decfistalogger, "Performed Fista for " << this->state()->currentIter() << " iterations");
      
      ASKAPLOG_INFO_STR(decfistalogger, this->control()->terminationString());
      
      this->finalise();

      absPeakVal=casa::max(casa::abs(this->residual()));

      this->state()->setPeakResidual(absPeakVal);
      this->state()->setObjectiveFunction(absPeakVal);

      return True;
    }
    
    template<class T, class FT>
    void DeconvolverFista<T,FT>::setBasisFunction(boost::shared_ptr<BasisFunction<T> > bf) {
      itsBasisFunction=bf;
    };
    
    template<class T, class FT>
    boost::shared_ptr<BasisFunction<T> > DeconvolverFista<T,FT>::basisFunction() {
      return itsBasisFunction;
    };
    
    // Apply the convolve operation - this is undecimated and redundant. The 2D image
    // will be expanded along the third axis
    template <class T, class FT>
    void DeconvolverFista<T, FT>::W(Array<T>& out, const Array<T>& in) {
      if(itsBasisFunction) {
	casa::Array<FT> inTransform(in.nonDegenerate().shape());
	casa::Array<FT> outPlaneTransform(in.nonDegenerate().shape());
	out.resize(itsBasisFunction->basisFunction().shape());
	casa::Cube<T> outCube(out);
	casa::setReal(inTransform, in.nonDegenerate());
	scimath::fft2d(inTransform, true);
	const uInt nPlanes(itsBasisFunction->basisFunction().shape()(2));
	for (uInt plane=0;plane<nPlanes;plane++) {
	  outPlaneTransform=inTransform.nonDegenerate()*Cube<FT>(itsBasisFunctionTransform).xyPlane(plane);
	  scimath::fft2d(outPlaneTransform, false);
	  outCube.xyPlane(plane)=real(outPlaneTransform);
	}
      }
      else {
	out=in.copy();
      }
    }

    // Apply the transpose of the W operation - this is undecimated and redundant so we need
    // to sum over the planes
    template <class T, class FT>
    void DeconvolverFista<T, FT>::WT(Array<T>& out, const Array<T>& in) {
      if(itsBasisFunction) {
	const Cube<T> inCube(in);
	casa::Array<FT> inPlaneTransform(out.nonDegenerate().shape());
	casa::Array<FT> outTransform(out.nonDegenerate().shape());
	outTransform.set(FT(0.0));

	// To reconstruct, we filter out each basis from the cumulative sum
	// and then add the corresponding term from the in array.
	const uInt nPlanes(itsBasisFunction->basisFunction().shape()(2));

	casa::setReal(inPlaneTransform, inCube.xyPlane(nPlanes-1));
	scimath::fft2d(inPlaneTransform, true);
	outTransform=Cube<FT>(itsBasisFunctionTransform).xyPlane(nPlanes-1)*(inPlaneTransform);

	for (uInt plane=1;plane<nPlanes;plane++) {
	  casa::setReal(inPlaneTransform, inCube.xyPlane(nPlanes-1-plane));
	  scimath::fft2d(inPlaneTransform, true);
	  outTransform=
	    outTransform+Cube<FT>(itsBasisFunctionTransform).xyPlane(nPlanes-1-plane)*(inPlaneTransform-outTransform);
	}
	scimath::fft2d(outTransform, false);
	out.nonDegenerate()=real(outTransform);
      }
      else {
	out=in.copy();
      }
    }

  } // namespace synthesis
  
} // namespace askap


