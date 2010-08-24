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

      ASKAPLOG_INFO_STR(decfistalogger, "Gain = " << this->control()->gain());

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

      Array<T> modelImage, modelImageTemp, modelImageOld;

      modelImageTemp.resize(this->model().shape());
      modelImageTemp.set(T(0.0));

      modelImageOld.resize(this->model().shape());
      modelImageOld.set(T(0.0));

      modelImage.resize(this->model().shape());
      modelImage.set(T(0.0));

      T absPeakVal;
      casa::IPosition absPeakPos;

      ASKAPLOG_INFO_STR(decfistalogger, "Performing Fista for " << this->control()->targetIter() << " iterations");

      updateResiduals(modelImage);

      modelImageTemp=this->residual()/this->itsLipschitz;

      absPeakVal=max(abs(this->residual()));

      T t_new=1;

      do {

        T aFit=sqrt(square(rms(this->residual()))+square(this->control()->fractionalThreshold()*absPeakVal));
        ASKAPLOG_INFO_STR(decfistalogger, "Scaling = " << aFit);

	modelImageOld=modelImageTemp.copy();

	T t_old=t_new;

	updateResiduals(modelImage);

	modelImage=modelImage+this->residual()/this->itsLipschitz;
	Array<T> D(this->model().shape());

	D=abs(modelImage+this->itsBackground)-aFit*this->control()->lambda()/this->itsLipschitz;
	modelImageTemp=D(D>T(0.0));
	modelImageTemp=sign(modelImage+this->itsBackground)*modelImageTemp;
	modelImageTemp-=this->itsBackground;

	t_new=(T(1.0)+sqrt(1+4*square(t_old)))/T(2.0);
	modelImage=modelImageTemp+((t_old-T(1.0))/t_new)*(modelImageTemp-modelImageOld);

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

        this->state()->setPeakResidual(absPeakVal);
	//        T l1Norm=sum(abs(modelImage+this->itsBackground));
        this->state()->setObjectiveFunction(absPeakVal);
        this->state()->setTotalFlux(sum(modelImageTemp)+sum(this->itsBackground));
        
        this->monitor()->monitor(*(this->state()));
        this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));
      this->model()=modelImageTemp.copy();
      
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


