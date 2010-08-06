/// @file
/// @brief Entropy operations as needed for Cornwell-Evans algorithm
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

ASKAP_LOGGER(decentropylogger, ".deconvolution.entropy");

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <string>

#include <casa/BasicMath/Math.h>

namespace askap {
  
  namespace synthesis {
    
    template<class T>
    EntropyBase<T>::EntropyBase() : itsAlpha(T(0.0)), itsBeta(T(0.0)), itsQ(T(40.0)) {
    };
    
    template<class T>
    EntropyBase<T>::~EntropyBase()
    {
    };
    
    template<class T>
    EntropyI<T>::~EntropyI() {
    };
    
    template<class T>
    EntropyI<T>::EntropyI() : EntropyBase<T>::EntropyBase() {
    };
    
    template<class T>
    T EntropyI<T>::formEntropy(const Array<T>& model)
    {
      T flux;
      flux = sum(this->itsMask * model);
      
      T myEntropy;
      
      myEntropy =  - sum(model * log( model / this->itsPrior ) );
      if (flux > 0.0) {
        myEntropy = myEntropy/flux + log(model.shape().product());
      } else {
        myEntropy = 0.0;
      }
      return myEntropy;
    };
    
    template<class T>
    Matrix<T> EntropyI<T>::formGDG(const Array<T>& model, const Array<T>& residual)
    {
      ASKAPCHECK(model.shape().conform(residual.shape()), "Model and residual images have different shapes");

      Matrix<T> GDG(4,4);
      GDG.set(0.0);
      
      T ggc = 2 * this->itsAlpha * this->itsQ;
      
      Array<T> rHess = model/(T(1.0) + ggc * model);
      Array<T> gradH = -log( model / (this->itsPrior));
      Array<T> gradC = -T(2.0)*residual;
      GDG(H,H) = sum(gradH * rHess * gradH);
      GDG(H,C) = sum(gradH * rHess * gradC);
      GDG(H,F) = sum(gradH * rHess);
      GDG(C,C) = sum(gradC * rHess * gradC);
      GDG(C,F) = sum(gradC * rHess);
      GDG(F,F) = sum(rHess);
      GDG(H,J) = GDG(H,H) -  this->itsAlpha * GDG(H,C) - this->itsBeta * GDG(H,F);
      GDG(C,J) = GDG(H,C) -  this->itsAlpha * GDG(C,C) - this->itsBeta * GDG(C,F);
      GDG(F,J) = GDG(H,F) -  this->itsAlpha * GDG(C,F) - this->itsBeta * GDG(F,F);
      GDG(J,J) = GDG(H,H) +  square(this->itsAlpha) * GDG(C,C) 
        + square(this->itsBeta)*GDG(F,F)  + 2*this->itsAlpha*this->itsBeta*GDG(C,F)  
        - 2*this->itsAlpha*GDG(H,C) - 2*this->itsBeta*GDG(H,F);
      return GDG;
    }
    
    template<class T>
    Matrix<T> EntropyI<T>::formGDGStep(const Array<T>& model, const Array<T>& residual, Array<T>& step)
    {
      ASKAPCHECK(model.shape().conform(residual.shape()), "Model and residual images have different shapes");

      Matrix<T> GDG(4,4);
      GDG.set(0.0);
      
      T ggc = 2 * this->itsAlpha * this->itsQ;
      
      Array<T> rHess = model/(T(1.0) + ggc * model);
      Array<T> gradH = -log(model) + log(this->itsPrior);
      Array<T> gradC = - T(2.0) *residual;
      Array<T> gradJ = gradH - this->itsAlpha*gradC - this->itsBeta;
      step = rHess * gradJ;
      GDG(H,H) = sum(gradH * rHess * gradH);
      GDG(H,C) = sum(gradH * rHess * gradC);
      GDG(H,F) = sum(gradH * rHess);
      GDG(C,C) = sum(gradC * rHess * gradC);
      GDG(C,F) = sum(gradC * rHess);
      GDG(F,F) = sum(rHess);
      GDG(H,J) = GDG(H,H) -  this->itsAlpha * GDG(H,C) - this->itsBeta * GDG(H,F);
      GDG(C,J) = GDG(H,C) -  this->itsAlpha * GDG(C,C) - this->itsBeta * GDG(C,F);
      GDG(F,J) = GDG(H,F) -  this->itsAlpha * GDG(C,F) - this->itsBeta * GDG(F,F);
      GDG(J,J) = GDG(H,H) +  square(this->itsAlpha) * GDG(C,C) 
        + square(this->itsBeta)*GDG(F,F)  + 2*this->itsAlpha*this->itsBeta*GDG(C,F)  
        - 2*this->itsAlpha*GDG(H,C) - 2*this->itsBeta*GDG(H,F);

      ASKAPCHECK(model.shape().conform(step.shape()), "Model and step images have different shapes");

      return GDG;
    }
    template<class T>
    T EntropyI<T>::formGDS(const Array<T>& model, const Array<T>& residual, const Array<T>& step)
    {
      ASKAPCHECK(model.shape().conform(step.shape()), "Model and step images have different shapes");
      ASKAPCHECK(model.shape().conform(residual.shape()), "Model and residual images have different shapes");

      T gds = sum(step * (-log( model/(this->itsPrior) ) + T(2.0) * this->itsAlpha * residual - this->itsBeta));
      return gds;
    };
    
    template<class T>
    void EntropyI<T>::configure(const LOFAR::ParameterSet &parset) {
    }

    template<class T>
    Emptiness<T>::Emptiness() : EntropyBase<T>::EntropyBase(), itsAFit(0.001) {
    };
    
    template<class T>
    Emptiness<T>::~Emptiness()
    {
    };
    
    template<class T>
    T Emptiness<T>::formEntropy(const Array<T>& model)
    {
      ASKAPCHECK(this->itsAFit>0.0, "Scaling in Emptiness is invalid");

      T flux = 0.0;
      flux = sum(model);
      T entropy = - sum( this->itsMask * (log(cosh((model - this->itsPrior)/this->itsAFit))) ) ;
      entropy = - this->itsAFit * entropy;
      return entropy;
    };

    template<class T>
    Matrix<T> Emptiness<T>::formGDG(const Array<T>& model, const Array<T>& residual)
    {
      ASKAPCHECK(model.shape().conform(residual.shape()), "Model and residual images have different shapes");
      ASKAPCHECK(this->itsAFit>0.0, "Scaling in Emptiness is invalid");

      Matrix<T> GDG(4,4);
      GDG.set(0.0);
      
      T ggc = 2 * this->itsAlpha * this->itsQ;
      
      Array<T> gradH = - tanh( (model - this->itsPrior)/this->itsAFit );
      Array<T> rHess = T(1.0)/( square(T(1.0)-gradH) /this->itsAFit + ggc) ;
      Array<T> gradC = - T(2.0)*residual;
      GDG(H,H) = sum(gradH * rHess * gradH);
      GDG(H,C) = sum(gradH * rHess * gradC);
      GDG(H,F) = sum(gradH * rHess);
      GDG(C,C) = sum(gradC * rHess * gradC);
      GDG(C,F) = sum(gradC * rHess);
      GDG(F,F) = sum(rHess);
      
      GDG(H,J) = GDG(H,H) -  this->itsAlpha * GDG(H,C) - this->itsBeta * GDG(H,F);
      GDG(C,J) = GDG(H,C) -  this->itsAlpha * GDG(C,C) - this->itsBeta * GDG(C,F);
      GDG(F,J) = GDG(H,F) -  this->itsAlpha * GDG(C,F) - this->itsBeta * GDG(F,F);
      GDG(J,J) = GDG(H,H) +  square(this->itsAlpha) * GDG(C,C) 
        + square(this->itsBeta)*GDG(F,F)  + 2*this->itsAlpha*this->itsBeta*GDG(C,F)  
        - 2*this->itsAlpha*GDG(H,C) - 2*this->itsBeta*GDG(H,F);
      return GDG;
    };
    
    
    template<class T>
    Matrix<T> Emptiness<T>::formGDGStep(const Array<T>& model, const Array<T>& residual,
                                         Array<T>& step)
    {
      ASKAPCHECK(this->itsAFit>0.0, "Scaling in Emptiness is invalid");
      ASKAPCHECK(model.shape().conform(residual.shape()), "Model and residual images have different shapes");
      
      Matrix<T> GDG(4,4);
      GDG.set(0.0);
      
      T ggc = 2 * this->itsAlpha * this->itsQ;
      
      Array<T> gradH = -tanh( (model - this->itsPrior)/this->itsAFit );
      Array<T> rHess = T(1.0)/( square(T(1.0)-gradH) /this->itsAFit + ggc) ;
      Array<T> gradC = -T(2.0)*residual;
      Array<T> gradJ = gradH - this->itsAlpha*gradC - this->itsBeta;
      step = rHess * gradJ;
      GDG(H,H) = sum(gradH * rHess * gradH);
      GDG(H,C) = sum(gradH * rHess * gradC);
      GDG(H,F) = sum(gradH * rHess);
      GDG(C,C) = sum(gradC * rHess * gradC);
      GDG(C,F) = sum(gradC * rHess);
      GDG(F,F) = sum(rHess);
      GDG(H,J) = GDG(H,H) -  this->itsAlpha * GDG(H,C) - this->itsBeta * GDG(H,F);
      GDG(C,J) = GDG(H,C) -  this->itsAlpha * GDG(C,C) - this->itsBeta * GDG(C,F);
      GDG(F,J) = GDG(H,F) -  this->itsAlpha * GDG(C,F) - this->itsBeta * GDG(F,F);
      GDG(J,J) = GDG(H,H) +  square(this->itsAlpha) * GDG(C,C) 
        + square(this->itsBeta)*GDG(F,F)  + 2*this->itsAlpha*this->itsBeta*GDG(C,F)  
        - 2*this->itsAlpha*GDG(H,C) - 2*this->itsBeta*GDG(H,F);

      ASKAPCHECK(model.shape().conform(step.shape()), "Model and step images have different shapes");

      return GDG;
    };
    
    
    template<class T>
    T Emptiness<T>::formGDS(const Array<T>& model, const Array<T>& residual, const Array<T>& step)
    {
      ASKAPCHECK(this->itsAFit>0.0, "Scaling in Emptiness is invalid");
      ASKAPCHECK(model.shape().conform(residual.shape()), "Model and residual images have different shapes");
      ASKAPCHECK(model.shape().conform(step.shape()), "Model and step images have different shapes");

      T gds = sum(this->itsMask*(step*(-tanh((model - this->itsPrior)/this->itsAFit)
                                       +T(2.0)*this->itsAlpha * residual - this->itsBeta)));
      return gds;
    };
    
    template<class T>
    void Emptiness<T>::configure(const LOFAR::ParameterSet &parset) {
    }

  } // namespace synthesis
  
} // namespace askap
