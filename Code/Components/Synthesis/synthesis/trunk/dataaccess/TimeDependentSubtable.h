/// @file
/// @brief A base class for handler of a time-dependent subtable
/// @details All classes representing time-dependent subtables are expected
/// to be derived from this one. It implements the method to 
/// convert a fully specified epoch into casa::Double intrinsically used by
/// the subtable. The actual subtable handler can use this for either 
/// an intelligent selection or efficient caching. The main idea behind this
/// class is to provide data necessary for a table
/// selection on the TIME column (which is a measure column). The class
/// reads units and the reference frame and sets up the converter.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TIME_DEPENDENT_SUBTABLE_H
#define TIME_DEPENDENT_SUBTABLE_H

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes
#include <measures/Measures/MEpoch.h>

// own includes
#include <dataaccess/ITimeDependentSubtable.h>
#include <dataaccess/IEpochConverter.h>

namespace conrad {

namespace synthesis {

/// @brief A base class for handler of a time-dependent subtable
/// @details All classes representing time-dependent subtables are expected
/// to be derived from this one. It implements the method to 
/// convert a fully specified epoch into casa::Double intrinsically used by
/// the subtable. The actual subtable handler can use this for either 
/// an intelligent selection or efficient caching. The main idea behind this
/// class is to provide data necessary for a table
/// selection on the TIME column (which is a measure column). The class
/// reads units and the reference frame and sets up the converter.
/// @ingroup dataaccess_tab
struct TimeDependentSubtable : virtual public ITimeDependentSubtable {

  /// @brief obtain time epoch in the subtable's native format
  /// @details Convert a given epoch to the table's native frame/units
  /// @param[in] time an epoch specified as a measure
  /// @return an epoch in table's native frame/units
  virtual casa::Double tableTime(const casa::MEpoch &time) const;
  
private:
  mutable boost::shared_ptr<IEpochConverter const> itsConverter;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TIME_DEPENDENT_SUBTABLE_H

