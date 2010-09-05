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
 
      ASKAPLOG_INFO_STR(decfistalogger, "Initialised FISTA solver");
   }

    template<class T, class FT>
    bool DeconvolverFista<T,FT>::deconvolve()
    {

      this->initialise();

      bool isMasked(this->itsWeightedMask.shape().conform(this->dirty().shape()));

      Array<T> X, X_old, X_temp;

      X_temp.resize(this->model().shape());
      X_temp.set(T(0.0));

      X_old.resize(this->model().shape());
      X_old.set(T(0.0));

      X.resize(this->model().shape());
      X=this->itsBackground.copy();

      T absPeakVal;
      casa::IPosition absPeakPos;

      ASKAPLOG_INFO_STR(decfistalogger, "Performing Fista for " << this->control()->targetIter() << " iterations");

      //      updateResidualsDouble(X);

      updateResiduals(X);

      X_temp=X.copy();

      absPeakVal=max(abs(this->residual()));

      T t_new=1;

      do {

	X_old=X_temp.copy();

	T t_old=t_new;

	updateResiduals(X);

	//	X=X+T(2.0)*this->residual()/this->itsLipschitz;
	X=X+this->residual()/this->itsLipschitz;

	// Transform to other (e.g. wavelet) space
	Array<T> WX(X);

	// Now shrink the coefficients towards zero and clip those below
	// lambda/lipschitz.
	Array<T> shrink(this->dirty().shape());
	{
	  Array<T> truncated(abs(WX)-this->control()->lambda()/this->itsLipschitz);
	  shrink=truncated(truncated>T(0.0));
	  shrink=sign(WX)*shrink;
	}

	// Transform back from other (e.g. wavelet) space here
	X_temp=shrink.copy();

	t_new=(T(1.0)+sqrt(T(1.0)+T(4.0)*square(t_old)))/T(2.0);

	X=X_temp+((t_old-T(1.0))/t_new)*(X_temp-X_old);

        {
	  casa::IPosition minPos;
          casa::IPosition maxPos;
          T minVal(0.0), maxVal(0.0);
          if (isMasked) {
            casa::minMaxMasked(minVal, maxVal, minPos, maxPos,
			       this->residual(),
                               itsWeightedMask);
          }
          else {
            casa::minMax(minVal, maxVal, minPos, maxPos, this->residual());
          }
          //
          ASKAPLOG_INFO_STR(decfistalogger, "   Maximum = " << maxVal << " at location " << maxPos);
          ASKAPLOG_INFO_STR(decfistalogger, "   Minimum = " << minVal << " at location " << minPos);
          if(abs(minVal)<abs(maxVal)) {
            absPeakVal=abs(maxVal);
            absPeakPos=maxPos;
          }
          else {
            absPeakVal=abs(minVal);
            absPeakPos=minPos;
          }
        }

	T l1Norm=sum(abs(X_temp));
	T fit=casa::sum(this->residual()*this->residual());
	T objectiveFunction(fit+this->control()->lambda()*l1Norm);
        this->state()->setPeakResidual(absPeakVal);
        this->state()->setObjectiveFunction(objectiveFunction);
        this->state()->setTotalFlux(sum(X_temp));
        
        this->monitor()->monitor(*(this->state()));
        this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));
      this->model()=X_temp.copy();
      
      ASKAPLOG_INFO_STR(decfistalogger, "Performed Fista for " << this->state()->currentIter() << " iterations");
      
      ASKAPLOG_INFO_STR(decfistalogger, this->control()->terminationString());
      
      this->finalise();

      absPeakVal=casa::max(casa::abs(this->residual()));

      this->state()->setPeakResidual(absPeakVal);
      this->state()->setObjectiveFunction(absPeakVal);

      return True;
    }
    
    template <class T, class FT>
    void DeconvolverFista<T, FT>::updateAlgorithm(Array<T>& delta, const Array<T>& model,
                                                  const Array<T>& residual, T aFit) {
      T scale=T(1.0)/(aFit);
      delta=(T(1.0)-exp(-pow(scale*residual,8)))*residual/this->itsLipschitz;
    }


  } // namespace synthesis
  
} // namespace askap


