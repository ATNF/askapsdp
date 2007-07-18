/// @file
/// @brief A base class for handlers of time-dependent subtables
/// @details All classes representing time-dependent subtables are expected
/// to be derived from this one. This interface provides a method to 
/// convert a fully specified epoch into casa::Double intrinsically used by
/// the subtable. The actual subtable handler can use this for either 
/// an intelligent selection or efficient caching. The main idea behind this
/// interface and derived classes is to provide data necessary for a table
/// selection on the TIME column (which is a measure column)
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TIME_DEPENDENT_SUBTABLE_H
#define I_TIME_DEPENDENT_SUBTABLE_H

// casa includes
#include <measures/Measures/MEpoch.h>

// own includes
#include <dataaccess/ITableHolder.h>

namespace conrad {

namespace synthesis {


/// @brief A base class for handlers of time-dependent subtables
/// @details All classes representing time-dependent subtables are expected
/// to be derived from this one. This interface provides a method to 
/// convert a fully specified epoch into casa::Double intrinsically used by
/// the subtable. The actual subtable handler can use this for either 
/// an intelligent selection or efficient caching. The main idea behind this
/// interface and derived classes is to provide data necessary for a table
/// selection on the TIME column (which is a measure column)
/// @ingroup dataaccess_tab
struct ITimeDependentSubtable : virtual protected ITableHolder {

  
  /// @brief obtain time epoch in the subtable's native format
  /// @details Convert a given epoch to the table's native frame/units
  /// @param[in] time an epoch specified as a measure
  /// @return an epoch in table's native frame/units
  virtual casa::Double tableTime(const casa::MEpoch &time) const = 0;
  
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_TIME_DEPENDENT_SUBTABLE_H

