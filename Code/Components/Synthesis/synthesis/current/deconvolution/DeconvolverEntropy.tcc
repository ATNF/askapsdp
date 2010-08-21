/// @file
/// @brief Class for a deconvolver based on entropy or min L1 norm
/// @details This concrete class defines a deconvolver used to estimate an
/// image from a residual image, psf optionally using a mask and a weights image.
/// @ingroup Deconvolver
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

#include <askap/AskapLogging.h>

ASKAP_LOGGER(decentropylogger, ".deconvolution.entropy");

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <string>

#include <deconvolution/DeconvolverEntropy.h>
#include <deconvolution/EntropyBase.h>
#include <deconvolution/EntropyI.h>
#include <deconvolution/Emptiness.h>

using namespace casa;

namespace askap {
  
  namespace synthesis {
    
    /// @brief Class for a deconvolver based on the Entropy algorithm of Cornwell and Evans
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a residual image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverEntropy<Double, DComplex>
    /// @ingroup Deconvolver
    
    template<class T, class FT>
    DeconvolverEntropy<T,FT>::DeconvolverEntropy(Array<T>& dirty, Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf)
    {
      this->model().resize(this->dirty().shape());
      this->model().set(T(0.0));
    };
    
    template<class T, class FT>
    DeconvolverEntropy<T,FT>::~DeconvolverEntropy() {
    };
    
    template<class T, class FT>
    void DeconvolverEntropy<T,FT>::finalise()
    {
      // Find residuals for current model model
      this->updateResiduals(this->itsModel);
    }
    
    template<class T, class FT>
    void DeconvolverEntropy<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();
    }
    
    // This is basically the algorithm described in the Cornwell-Evans paper of 1985, with
    // some improvements as suggested by Bob Sault.
    template<class T, class FT>
    bool DeconvolverEntropy<T,FT>::deconvolve()
    {
      
      this->initialise();
      
      ASKAPLOG_INFO_STR(decentropylogger, "Performing Entropy deconvolution for "
                        << this->control()->targetIter() << " iterations");

      ASKAPLOG_INFO_STR(decentropylogger, "Target rms fit = " << this->control()->targetObjectiveFunction());

      uInt numberPixels = this->itsModel.shape().product();
      T targetChisq = square(this->control()->targetObjectiveFunction()) * numberPixels;
      T chisq;
      T fit;

      // Assume that the dirty image can be scaled and used as an initial model
      Array<T> trialModel(this->model().shape());
      this->itsModel=this->dirty()/this->itsLipschitz;
      this->updateResiduals(this->itsModel);
        
      Array<T> step(this->model().shape());
      step.set(T(0.0));
        

      do {
        // Find the current fit
        chisq = sum(square(this->residual()));  
        fit = sqrt(chisq/targetChisq);

        T aFit = max(abs(this->residual()))/this->itsLipschitz;
        //        ASKAPLOG_INFO_STR(decentropylogger, "Scaling = " << aFit << " Jy/pixel");
        this->itsEntropy->setScale(aFit);

        Matrix<T> GDG(this->itsEntropy->formGDGStep(this->itsModel, this->itsResidual, step));

        // Check to see if Alpha and Beta need to be initialised. If so then we need to
        // do so and recalculate the gradients and step
        if(this->itsEntropy->initialiseAlphaBeta(GDG)) {
          GDG = this->itsEntropy->formGDGStep(this->itsModel, this->itsResidual, step);
        }

        
        T flux=sum(this->itsModel);
        this->itsEntropy->changeAlphaBeta(GDG, targetChisq, chisq, this->control()->targetFlux(), flux);
        
        // Now find the normalised gradient - we will use this to limit the step taken
        T length(this->itsEntropy->formLength(GDG));
        if (length <= T(0.0)) {
          length = GDG(F,F);
        }
        T normGrad = GDG(J,J) / length;

        //        relaxMin();
      
        // We limit the step to less than the tolerance e.g. 0.1 so that the
        // quadratic approximation in the Newton-Raphson is still valid.
        T scale = 1.0;
        T scalem = 1.0;
        if (normGrad > 0.0) {
          scalem = this->control()->tolerance()/normGrad;
        }
        scale = min(T(1.0), scalem);
        
        // OK - now we take the proposed step and evaluate the
        // gradient there.
        trialModel=this->itsModel + scale * step;
      
        // Calculate residual for this new trial image
        this->updateResiduals(trialModel);
        chisq = rms(this->residual());  
        
        // Form the scalar Gradient . Step at this new location. Ideally this should be
        // zero. Once we know the value of the gradient initially and for the trial image, we
        // can determine the optimal step by interpolating the gradient to zero. Ideally the
        // step should be O(1) times the original step
        T eps = 1.0;
        T gradDotStep0 = GDG(J,J);
        T gradDotStep1(this->itsEntropy->formGDS(this->itsModel, this->itsResidual, step));

        if (gradDotStep0 != gradDotStep1) {
          eps = gradDotStep0/(gradDotStep0-gradDotStep1);
        }
        if (scale != T(0.0)) eps = min(eps, T(scalem/scale) );
        if (eps <= T(0.0)) {
          eps = T(1.0);
        }

        // Step to optimum point
        this->itsModel=this->itsModel + scale * eps * step;

          // Recalculate residual for the new image
        updateResiduals(this->itsModel);
        chisq = rms(this->itsResidual);
      
      // readjust beam volume
      //      itsQ = itsQ*(T(1.0)/max(T(0.5), min(T(2.0),eps))+T(1.0))/T(2.0);
      
        flux=sum(this->model());
        this->itsEntropy->changeAlphaBeta(GDG, targetChisq, chisq, this->control()->targetFlux(), flux);
        
        T absPeakVal(max(abs(this->itsResidual)));

        this->state()->setPeakResidual(absPeakVal);
        this->state()->setObjectiveFunction(sqrt(chisq/numberPixels));
        this->state()->setTotalFlux(flux);
        
        this->monitor()->monitor(*(this->state()));
        this->state()->incIter();
    }
    while (!this->control()->terminate(*(this->state())));
    
    ASKAPLOG_INFO_STR(decentropylogger, "Performed Entropy deconvolution for "
                      << this->state()->currentIter() << " iterations");
    
    ASKAPLOG_INFO_STR(decentropylogger, this->control()->terminationString());
    
    this->finalise();
    
    return True;
  }
  
    template<class T, class FT>
    void DeconvolverEntropy<T,FT>::configure(const LOFAR::ParameterSet& parset)
    {        

      String algorithm(parset.getString("algorithm", "Emptiness"));
      this->control()->setAlgorithm(algorithm);

      if(algorithm=="EntropyI") {
        ASKAPLOG_INFO_STR(decentropylogger, "Maximising information entropy of model image");
	itsEntropy=boost::shared_ptr<EntropyBase<T> >(new EntropyI<T>());
        itsEntropy->setTolerance(parset.getFloat("tolerance", 0.3));
      }
      else {
        ASKAPLOG_INFO_STR(decentropylogger, "Maximising emptiness (negative L1 norm) of model image");
	itsEntropy=boost::shared_ptr<EntropyBase<T> >(new Emptiness<T>());
        itsEntropy->setTolerance(parset.getFloat("tolerance", 0.3));
      }

    }

  } // namespace synthesis

} // namespace askap


