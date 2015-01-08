/// @file 
/// @brief Basic processing step which does nothing
/// @details This is a simple processing step which does no useful work
/// (i.e. essentially no operation stub). It only sets the name of the
/// object and implements the appropriate getter method.
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

// own includes
#include "processingsteps/ProcessingStep.h"

// for logging
#include "askap_parallelframework.h"
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallelframework");

namespace askap {

namespace askapparallel {
	
/// @brief an empty constructor to create unnamed object
ProcessingStep::ProcessingStep() : itsName("unnamed") {}

/// @brief construct an object and assign name
/// @details 
/// @param[in] name name to assign
ProcessingStep::ProcessingStep(const std::string &name) :
     itsName(name) {}

/// @brief initialisation of the processing steps
/// @details Processing steps are required not to do any heavy
/// initialisation, caching or memory allocation in constructors.
/// Override this method for this type of things.
void ProcessingStep::initialise(const IContext &) {}

/// @brief core of the processing step
/// @details Override this method in derived classes with 
/// the actual implementation of the processing algorithm.
/// @param[in] context a reference to context class
void ProcessingStep::run(IContext &context)
{
   // in all practical cases this method will be overridden,
   // so can use INFO instead of DEBUG
   ASKAPLOG_INFO_STR(logger, "Empty processing step ("<<
          name()<<"): iteration initialised = "<<
          context.hasMore()<<" global rank = "<<
          context.globalComm()->rank());
}

/// @brief finalisation of the processing step
/// @details If some resourses have been allocated in
/// initalise method, this is the place to release them
void ProcessingStep::finalise(const IContext &) {}


/// @brief obtain the name of this step
/// @details For logging and cross-reference it is handy to
/// assign a name to processing steps. This method returns
/// the name.
/// @return name of the step, "unnamed" is returned for an
/// object constructed with the default constructor
std::string ProcessingStep::name() const
{
  return itsName;
}

} // end of namespace askapparallel
} // end of namespace askap

