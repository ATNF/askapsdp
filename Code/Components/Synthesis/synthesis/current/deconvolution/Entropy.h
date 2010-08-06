/// @file
/// @brief Base class for a deconvolver
/// @details This interface class defines a deconvolver used to estimate an
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

#ifndef I_SYNTHESIS_ENTROPY_H
#define I_SYNTHESIS_ENTROPY_H

#include <string>

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Array.h>

#include <Common/ParameterSet.h>

using namespace casa;

namespace askap {
  
  namespace synthesis {
    
    // Base class
    template<class T>
    class EntropyBase {
    public:
      
      typedef boost::shared_ptr<EntropyBase<T> > ShPtr;

      enum GRADTYPE {H=0, C, F, J };

      // Construct the basis class
      EntropyBase();

      // A virtual destructor may be necessary for use in derived classes.
      virtual ~EntropyBase();
      
      // calculate the entropy for the whole image
      virtual T formEntropy(const Array<T>& model) {return 0;};
      
      // calculate the Gradient dot Gradient matrix
      virtual Matrix<T> formGDG(const Array<T>& model, const Array<T>& residual) {return Matrix<Float>(1,1);};
      
      // calculate the Gradient dot Gradient matrix, calculate Step
      virtual Matrix<T> formGDGStep(const Array<T>& model, const Array<T>& residual,
                                    Array<T>& step) {return Matrix<Float>(1,1);};
      
      // calculate Gradient dot Step
      virtual T formGDS(const Array<T>& model, const Array<T>& residual, const Array<T>& step) {return T(0);};
      
      /// @brief configure basic parameters of the solver
      /// @details This method encapsulates extraction of basic solver parameters from the parset.
      /// @param[in] parset parset
      virtual void configure(const LOFAR::ParameterSet &parset) {};
      
      void setAlpha(const T alpha) {itsAlpha=alpha;};

      void setBeta(const T beta) {itsBeta=beta;};

      void setQ(const T Q) {itsQ=Q;};

      void setMask(const Array<T>& mask) {itsMask=mask.copy();};

      void setPrior(const Array<T>& prior) {itsPrior=prior.copy();};

    protected:
      
      T itsAlpha;
      
      T itsBeta;

      T itsQ;
      
      Array<T> itsMask;

      Array<T> itsPrior;
    };
    
    
    // Thermodynamic or Information entropy
    template<class T>
    class EntropyI : public EntropyBase<T> {
    public:
      
      typedef boost::shared_ptr<EntropyI<T> > ShPtr;

      enum GRADTYPE {H=0, C, F, J };

      // Construct the class
      EntropyI();

      // A virtual destructor may be necessary for use in derived classes.
      virtual ~EntropyI();
      
      // calculate the entropy for the whole image
      virtual T formEntropy(const Array<T>& model) ;
      
      // calculate the Gradient dot Gradient matrix
      virtual Matrix<T> formGDG(const Array<T>& model, const Array<T>& residual);
      
      // calculate the Gradient dot Gradient matrix, calculate Step
      virtual Matrix<T> formGDGStep(const Array<T>& model, const Array<T>& residual, Array<T>& step);
      
      // calculate Gradient dot Step
      virtual T formGDS(const Array<T>& model, const Array<T>& residual, const Array<T>& step);
      
      /// @brief configure basic parameters of the solver
      /// @details This method encapsulates extraction of basic solver parameters from the parset.
      /// @param[in] parset parset
      virtual void configure(const LOFAR::ParameterSet &parset) ; 
      
    protected:

    };
    
    
    // <summary> Maximum Emptiness measure used by MEM
    // </summary>
    
    // Emptiness measure
    template<class T>
    class Emptiness : public EntropyBase<T>
    {
    public:
      
      typedef boost::shared_ptr<Emptiness<T> > ShPtr;

      enum GRADTYPE {H=0, C, F, J };

      // Construct the class
      Emptiness();

      // A virtual destructor may be necessary for use in derived classes.
      virtual ~Emptiness();
      
      // calculate the entropy for the whole image
      virtual T formEntropy(const Array<T>& model);
      
      // calculate the Gradient dot Gradient matrix
      virtual Matrix<T> formGDG(const Array<T>& model, const Array<T>& residual);
      
      // calculate the Gradient dot Gradient matrix, calculate Step
      virtual Matrix<T> formGDGStep(const Array<T>& model, const Array<T>& residual, Array<T>& step);
      
      // calculate Gradient dot Step
      virtual T formGDS(const Array<T>& model, const Array<T>& residual, const Array<T>& step);
      
      /// @brief configure basic parameters of the solver
      /// @details This method encapsulates extraction of basic solver parameters from the parset.
      /// @param[in] parset parset
      virtual void configure(const LOFAR::ParameterSet &parset); 
      
    protected:

      T itsAFit;
    };
    
  } // namespace synthesis
  
} // namespace askap

#include <deconvolution/Entropy.tcc>

#endif
