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

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>

#include <casa/aips.h>
#include <deconvolution/DeconvolverState.h>
#include <deconvolution/DeconvolverControl.h>

using namespace casa;

namespace askap {
  
  namespace synthesis {
    
    
    template<class T>
    DeconvolverControl<T>::DeconvolverControl() :
      itsTerminationCause(NOTTERMINATED), itsTargetIter(0),
      itsTargetObjectiveFunction(T(0)), itsGain(1.0), itsTolerance(1e-4) 
    {};
    
    /// Control the current state
    template<class T>
    Bool DeconvolverControl<T>::terminate(const DeconvolverState<T>& state) {
      // Check for convergence
      if(abs(state.objectiveFunction())<itsTargetObjectiveFunction) {
        ASKAPLOG_INFO_STR(logger, "Objective function " << state.objectiveFunction()
                          << " less than target " << itsTargetObjectiveFunction);
        itsTerminationCause = CONVERGED;
        return True;
      }
      // Check for too many iterations
      if((state.currentIter()>-1)&&(this->targetIter()>0)&&(state.currentIter()>=this->targetIter())) {
        itsTerminationCause = EXCEEDEDITERATIONS;
        return True;
      }
      return False;
    }
    template<class T>
    String DeconvolverControl<T>::terminationString() const {
      switch (itsTerminationCause) {
      case CONVERGED:
        return String("Converged");
        break;
      case DIVERGED:
        return String("Diverged");
        break;
      case EXCEEDEDITERATIONS:
        return String("Exceeded maximum number of iterations");
        break;
      case NOTTERMINATED:
        return String("Not yet terminated");
        break;
      case UNKNOWN:
        return String("Termination for unknown reason");
        break;
      default:
        return String("Logic error in termination");
        break;
      }
    }
    
  } // namespace synthesis
  
} // namespace askap
