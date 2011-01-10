/// @file
///
/// Provides generic methods for parallel algorithms
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
#ifndef ASKAP_SYNTHESIS_SYNPARALLEL_H_
#define ASKAP_SYNTHESIS_SYNPARALLEL_H_

#include <fitting/Equation.h>
#include <fitting/Solver.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>
#include <Common/ParameterSet.h>


#include <askapparallel/AskapParallel.h>

namespace askap
{
  namespace synthesis
  {
    /// @brief Support for parallel algorithms 
    ///
    /// @details Support for parallel applications in the area.
    /// An application is derived from this abstract base. The model used is that the
    /// application has many workers and one master, running in separate MPI processes
    /// or in one single thread. The master is the master so the number of processes
    /// is one more than the number of workers. 
    ///
    /// If the number of nodes is 1 then everything occurs in the same process with
    /// no overall for transmission of model.
    ///
    /// @ingroup parallel
    class SynParallel
    {
  public:

      /// @brief Constructor 
      /// @details The first parameter is needed solely for MPI, the second
      /// is the parset to be used in derived classes
      /// @param[in] comms communications object
      /// @param[in] parset parameter set      
      SynParallel(askap::mwbase::AskapParallel& comms, const LOFAR::ParameterSet& parset);

      ~SynParallel();

      /// Return the model
      askap::scimath::Params::ShPtr& params();

      /// @brief Broadcast the model to all workers
      void broadcastModel();

      /// @brief Receive the model from the master
      void receiveModel();

      /// Substitute %w by worker number, and %n by number of workers (one less than the number
      // of nodes). This allows workers to do different work! This just calls
      // through to the AskapParallel version of substitute()
      std::string substitute(const std::string& s) const;

  protected:
      /// @brief obtain parameter set
      /// @details to be used in derived classes
      /// @return reference to the parameter set object
      inline const LOFAR::ParameterSet& parset() const { return itsParset;}

      /// The model
      askap::scimath::Params::ShPtr itsModel;

      /// Class for communications
      askap::mwbase::AskapParallel& itsComms;
  private:
      /// @brief parameter set to get the parameters from
      LOFAR::ParameterSet itsParset;
          
    };

  }
}
#endif
