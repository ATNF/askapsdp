/// @file 
/// @brief Processing step interface
/// @details New parallel framework treats all processing as a collection
/// of processing steps which can be parallelised or run sequentially.
/// This interface is a common base class for all processing steps.
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

#ifndef ASKAP_ASKAPPARALLEL_I_PROCESSING_STEP_H
#define ASKAP_ASKAPPARALLEL_I_PROCESSING_STEP_H

// own includes
#include "application/IContext.h"

// std includes
#include <string>

namespace askap {

namespace askapparallel {
	
/// @brief Processing step interface
/// @details New parallel framework treats all processing as a collection
/// of processing steps which can be parallelised or run sequentially.
/// This interface is a common base class for all processing steps.
class IProcessingStep
{
public:
	
  /// an empty virtual destructor to make the compiler happy
  virtual ~IProcessingStep();

  /// @brief initialisation of the processing steps
  /// @details Processing steps are required not to do any heavy
  /// initialisation, caching or memory allocation in constructors.
  /// Override this method for this type of things.
  /// @param[in] context a reference to context class
  /// @note the only non-const methods in the context are
  /// related to work domain iteration. These are not supposed to
  /// be used in initialise. Therefore, passing parameter by
  /// const reference.
  virtual void initialise(const IContext &context) = 0;

  /// @brief core of the processing step
  /// @details Override this method in derived classes with 
  /// the actual implementation of the processing algorithm.
  /// @param[in] context a reference to context class
  virtual void run(IContext &context) = 0;

  /// @brief finalisation of the processing step
  /// @details If some resourses have been allocated in
  /// initalise method, this is the place to release them
  /// @param[in] context a reference to context class
  /// @note the only non-const methods in the context are
  /// related to work domain iteration. These are not supposed to
  /// be used in initialise. Therefore, passing parameter by
  /// const reference.
  virtual void finalise(const IContext &context) = 0;


  /// @brief obtain the name of this step
  /// @details For logging and cross-reference it is handy to
  /// assign a name to processing steps. This method returns
  /// the name.
  /// @return name of the step
  virtual std::string name() const = 0;
};

} // end of namespace askapparallel
} // end of namespace askap
#endif // ASKAP_ASKAPPARALLEL_I_PROCESSING_STEP_H

