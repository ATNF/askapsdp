/// @file
/// @brief A handler of  ANTENNA subtable
/// @details This class provides access to the ANTENNA subtable (which contains 
/// antenna mounts and positions for all antennas). It caches the whole table
/// in constructor and then returns cached values. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef MEM_ANTENNA_SUBTABLE_HANLDER_H
#define MEM_ANTENNA_SUBTABLE_HANLDER_H

// casa includes
#include <tables/Tables/Table.h>
#include <casa/Arrays/Vector.h>

// own includes
#include <dataaccess/IAntennaSubtableHandler.h>

namespace conrad {

namespace synthesis {

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
private:
  /// a cache of antenna mounts
  casa::Vector<casa::String> itsMounts;
  
  /// a cache of antenna positions
  casa::Vector<casa::MPosition> itsPositions;   
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef MEM_ANTENNA_SUBTABLE_HANLDER_H
