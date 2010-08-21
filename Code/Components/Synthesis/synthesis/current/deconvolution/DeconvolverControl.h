/// @file
/// @brief Base class for Control of Deconvolver
/// @details All the Controling is delegated to this class so that
/// more control is possible.
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

#ifndef I_DECONVOLVERCONTROL_H
#define I_DECONVOLVERCONTROL_H
#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverState.h>

#include <Common/ParameterSet.h>

using namespace casa;

namespace askap {
  
  namespace synthesis {
    
    /// @brief Base class for Control of Deconvolver
    /// @details All the controlling is delegated to this class so that
    /// more control is possible.
    /// @ingroup Deconvolver
    
    template<typename T> class DeconvolverControl {
      
    public:
      typedef boost::shared_ptr<DeconvolverControl<T> > ShPtr;
      
      enum TerminationCause {
    	CONVERGED,
        DIVERGED,
        EXCEEDEDITERATIONS,
        NOTTERMINATED,
        UNKNOWN
      };
      
      DeconvolverControl();
      
      virtual ~DeconvolverControl() {};
      
      /// @brief configure basic parameters of the solver
      /// @details This method encapsulates extraction of basic solver parameters
      /// from the parset.
      /// @param[in] parset parset
      virtual void configure(const LOFAR::ParameterSet &parset); 

      /// Terminate?
      Bool terminate(const DeconvolverState<T>& ds);
      
      /// Termination cause returned as String
      String terminationString() const;
      
      /// Set the termination cause
      void setTerminationCause(const TerminationCause cause) {
        itsTerminationCause=cause;
      };
      
      /// Termination cause returned as an enum
      TerminationCause terminationCause() const {
    	return itsTerminationCause;
      };
      
      /// Algorithm name returned as String
      String algorithm() const {return itsAlgorithm;};
      
      /// Set the termination cause
      void setAlgorithm(const String algorithm) {
        itsAlgorithm=algorithm;
      };
      
      void setGain(Float gain)
      {
        itsGain = gain;
      }

      Float gain() const
      {
        return itsGain;
      }

      void setTolerance(Float tolerance)
      {
        itsTolerance = tolerance;
      }

      Float tolerance() const
      {
        return itsTolerance;
      }

      void setTargetIter(Int targetIter)
      {
        itsTargetIter = targetIter;
      }

      Int targetIter() const
      {
        return itsTargetIter;
      }

      void setLambda(T lambda)
      {
        itsLambda = lambda;
      }

      T lambda() const
      {
        return itsLambda;
      }

      void setTargetObjectiveFunction(T objectiveFunction) {
    	itsTargetObjectiveFunction = objectiveFunction;
      }
      
      T targetObjectiveFunction() const {
    	return itsTargetObjectiveFunction;
      }

      void setTargetFlux(T targetFlux) {
    	itsTargetFlux = targetFlux;
      }
      
      T targetFlux() const {
    	return itsTargetFlux;
      }

      void setFractionalThreshold(Float fractionalThreshold) {
    	itsFractionalThreshold=fractionalThreshold;
      }
      
      Float fractionalThreshold() {
    	return itsFractionalThreshold;
      }

      /// @brief Set the desired PSF width in pixels
      void setPSFWidth(const Int psfWidth) {itsPSFWidth=psfWidth;}

      /// @brief Get the desired PSF width in pixels
      Int psfWidth() const {return itsPSFWidth;};

    private:
      String itsAlgorithm;
      TerminationCause itsTerminationCause;
      Int itsTargetIter;
      T itsTargetObjectiveFunction;
      T itsTargetFlux;
      Float itsFractionalThreshold;
      Float itsGain;
      Float itsTolerance;
      Int itsPSFWidth;
      T itsLambda;
    };
    
  } // namespace synthesis
  
} // namespace askap

#include <deconvolution/DeconvolverControl.tcc>

#endif


