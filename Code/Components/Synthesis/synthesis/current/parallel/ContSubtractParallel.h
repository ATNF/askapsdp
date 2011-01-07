/// @file
///
/// ContSubtractParallel: Support for parallel continuum subtraction using model
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef CONT_SUBTRACT_PARALLEL_H
#define CONT_SUBTRACT_PARALLEL_H

// ASKAPsoft includes
#include <Common/ParameterSet.h>
#include <parallel/MEParallelApp.h>

namespace askap {

namespace synthesis {

/// @brief parallel helper for continuum subtraction
/// @details This class does the core operation to subtract continuum by doing visibility
/// prediction from the given model in parallel.
/// @ingroup parallel
class ContSubtractParallel : public MEParallelApp 
{
public:
   /// @brief Constructor from ParameterSet
   /// @details The parset is used to construct the internal state. We could
   /// also support construction from a python dictionary (for example).
   /// The command line inputs are needed solely for MPI - currently no
   /// application specific information is passed on the command line.
   /// @param comms communication object 
   /// @param parset ParameterSet for inputs
   ContSubtractParallel(askap::mwbase::AskapParallel& comms, const LOFAR::ParameterSet& parset);
   
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef CONT_SUBTRACT_PARALLEL_H


