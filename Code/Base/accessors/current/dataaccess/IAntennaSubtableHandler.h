/// @file
/// @brief An interface to ANTENNA subtable
/// @details A class derived from this interface provides access to
/// the content of the ANTENNA subtable (which provides antenna mounts and
/// positions). It looks like the measurement set can't easily handle 
/// time-dependent antenna tables and this case is definitely out of scope for
/// ASKAP. Therefore, the interface doesn't allow the information to change in
/// time. 
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

#ifndef I_ANTENNA_SUBTABLE_HANLDER_H
#define I_ANTENNA_SUBTABLE_HANLDER_H

// casa includes
#include <measures/Measures/MPosition.h>
#include <casa/BasicSL/String.h>

// own includes
#include <dataaccess/IHolder.h>

namespace askap {

namespace accessors {

/// @brief An interface to ANTENNA subtable
/// @details A class derived from this interface provides access to
/// the content of the ANTENNA subtable (which provides antenna mounts and
/// positions). It looks like the measurement set can't easily handle 
/// time-dependent antenna tables and this case is definitely out of scope for
/// ASKAP. Therefore, the interface doesn't allow the information to change
/// with time. 
/// @ingroup dataaccess_tab
struct IAntennaSubtableHandler : virtual public IHolder {
  
  /// @brief obtain the position of the given antenna
  /// @details
  /// @param[in] antID antenna ID to return the position for
  /// @return a reference to the MPosition measure
  virtual const casa::MPosition& getPosition(casa::uInt antID) const = 0;
  
  /// @brief obtain the mount type for the given antenna
  /// @details
  /// @param[in] antID antenna ID to return the position for
  /// @return a string describing the mount type
  virtual const casa::String& getMount(casa::uInt antID) const = 0;
  
  /// @brief check whether all antennae are equatorialy mounted
  /// @details
  /// This method checks the mount type for all antennas to be 
  /// either EQUATORIAL or equatorial. This mount type doesn't require
  /// parallactic angle rotation and can be trated separately.
  /// @return true, if all antennae are equatorially mounted
  virtual bool allEquatorial() const = 0; 
  
  /// @brief get the number of antennae
  /// @details
  /// This method returns the number of antennae (i.e. all antID indices
  /// are expected to be less than this number). Following the general
  /// assumptions about ANTENNA subtable, this number is assumed to be
  /// fixed.
  /// @return total number of antennae 
  virtual casa::uInt getNumberOfAntennae() const = 0;
};


} // namespace accessors

} // namespace askap

#endif // #ifndef I_ANTENNA_SUBTABLE_HANLDER_H
