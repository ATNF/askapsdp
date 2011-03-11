/// @file
/// @brief A handler of  ANTENNA subtable
/// @details This class provides access to the ANTENNA subtable (which contains 
/// antenna mounts and positions for all antennas). It caches the whole table
/// in constructor and then returns cached values. 
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

#ifndef MEM_ANTENNA_SUBTABLE_HANLDER_H
#define MEM_ANTENNA_SUBTABLE_HANLDER_H

// casa includes
#include <tables/Tables/Table.h>
#include <casa/Arrays/Vector.h>

// own includes
#include <dataaccess/IAntennaSubtableHandler.h>

namespace askap {

namespace accessors {

/// @brief A handler of  ANTENNA subtable
/// @details This class provides access to the ANTENNA subtable (which contains 
/// antenna mounts and positions for all antennas). It caches the whole table
/// in constructor and then returns cached values. 
/// @ingroup dataaccess_tab
struct MemAntennaSubtableHandler : virtual public IAntennaSubtableHandler {
  
  /// read all required information from the ANTENNA subtable
  /// @param[in] ms an input measurement set (a table which has an
  /// ANTENNA subtable)
  explicit MemAntennaSubtableHandler(const casa::Table &ms);
  
  /// @brief obtain the position of the given antenna
  /// @details
  /// @param[in] antID antenna ID to return the position for
  /// @return a reference to the MPosition measure
  virtual const casa::MPosition& getPosition(casa::uInt antID) const;
  
  /// @brief obtain the mount type for the given antenna
  /// @details
  /// @param[in] antID antenna ID to return the position for
  /// @return a string describing the mount type
  virtual const casa::String& getMount(casa::uInt antID) const;
  
  /// @brief check whether all antennae are equatorialy mounted
  /// @details
  /// This method checks the mount type for all antennas to be 
  /// either EQUATORIAL or equatorial. This mount type doesn't require
  /// parallactic angle rotation and can be trated separately.
  /// @return true, if all antennae are equatorially mounted
  virtual bool allEquatorial() const throw();   
  
  /// @brief get the number of antennae
  /// @details
  /// This method returns the number of antennae (i.e. all antID indices
  /// are expected to be less than this number). Following the general
  /// assumptions about ANTENNA subtable, this number is assumed to be
  /// fixed.
  /// @return total number of antennae 
  virtual casa::uInt getNumberOfAntennae() const;
    
private:
  /// a cache of antenna mounts
  casa::Vector<casa::String> itsMounts;
  
  /// a cache of antenna positions
  casa::Vector<casa::MPosition> itsPositions;
  
  /// an internal buffer for a flag showing whether all antennae are equatorially
  /// mounted
  bool itsAllEquatorial;   
};


} // namespace accessors

} // namespace askap

#endif // #ifndef MEM_ANTENNA_SUBTABLE_HANLDER_H
