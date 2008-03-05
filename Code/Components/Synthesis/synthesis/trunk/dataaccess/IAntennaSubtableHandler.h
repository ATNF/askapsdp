/// @file
/// @brief An interface to ANTENNA subtable
/// @details A class derived from this interface provides access to
/// the content of the ANTENNA subtable (which provides antenna mounts and
/// positions). It looks like the measurement set can't easily handle 
/// time-dependent antenna tables and this case is definitely out of scope for
/// ASKAP. Therefore, the interface doesn't allow the information to change in
/// time. 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
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

namespace synthesis {

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
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef I_ANTENNA_SUBTABLE_HANLDER_H
