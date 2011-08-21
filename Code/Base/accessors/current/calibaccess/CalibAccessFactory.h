/// @file
/// @brief factory creating calibration parameter accessor
/// @details This factory creates an actual instance of the calibration
/// parameter accessor and returns a generic instance via shared pointer.
/// Different implementations are possible: parset-based, casa table-based,
/// ICE-based. We could even load the actual code from a shared library as it
/// is done for gridders (may help to break dependencies). For now this factory
/// method is in calibaccess, but can be moved somewhere else later, especially
/// when an ICE-based implementation is ready to be plugged in.
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef CALIB_ACCESS_FACTORY_H
#define CALIB_ACCESS_FACTORY_H

#include <calibaccess/ICalSolutionConstSource.h>
#include <calibaccess/ICalSolutionSource.h>

#include <boost/shared_ptr.hpp>

#include <Common/ParameterSet.h>

namespace askap {

namespace accessors {

/// @brief factory creating calibration parameter accessor
/// @details This factory creates an actual instance of the calibration
/// parameter accessor and returns a generic instance via shared pointer.
/// Different implementations are possible: parset-based, casa table-based,
/// ICE-based. We could even load the actual code from a shared library as it
/// is done for gridders (may help to break dependencies). For now this factory
/// method is in calibaccess, but can be moved somewhere else later, especially
/// when an ICE-based implementation is ready to be plugged in.
/// @note Factory methods are made static for now, but in general the factory could
/// have a state (and the parset/configuration could be supplied in the constructor)
/// @ingroup calibaccess
struct CalibAccessFactory {
   
   /// @brief Build an appropriate "calibration source" class
   /// @details This is a factory method generating a shared pointer to the calibration
   /// solution source according to the parset file which allows write operation.
   /// @param[in] parset parameters containing description of the class to be constructed 
   /// (without leading Cimager., etc)
   /// @return shared pointer to the calibration solution source object
   static boost::shared_ptr<ICalSolutionSource> rwCalSolutionSource(const LOFAR::ParameterSet &parset);
   
   /// @brief Build an appropriate "calibration source" class
   /// @details This is a factory method generating a shared pointer to the calibration
   /// solution source according to the parset file which allows read operation only.
   /// @param[in] parset parameters containing description of the class to be constructed
   /// @return shared pointer to the calibration solution source object
   /// @note For now we have the same implementation for write and for read and therefore implement
   /// read-only method via read-write one.
   inline static boost::shared_ptr<ICalSolutionConstSource> roCalSolutionSource(const LOFAR::ParameterSet &parset)
      { return rwCalSolutionSource(parset); }
};

} // namespace accessors

} // namespace askap

#endif // #ifndef CALIB_ACCESS_FACTORY_H
