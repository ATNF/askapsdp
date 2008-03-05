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
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TIME_DEPENDENT_SUBTABLE_H
#define TIME_DEPENDENT_SUBTABLE_H

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes
#include <measures/Measures/MEpoch.h>
#include <casa/BasicSL/String.h>

// own includes
#include <dataaccess/ITimeDependentSubtable.h>
#include <dataaccess/IEpochConverter.h>

namespace askap {

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
  
  /// @brief obtain a full epoch object for a given time (reverse conversion)
  /// @details Some subtables can have more than one time-related columns, i.e.
  /// TIME and INTERVAL. This method allows to form a full MEpoch measure from
  /// the time represented as double in the native table's reference frame/unit.
  /// It allows to extract frame/unit information and compare them with that of
  /// the other columns.
  /// @param[in] time time to translate into a full epoch
  /// @return[in] full epoch corresponding to a given time
  virtual casa::MEpoch tableTime(casa::Double time) const;

protected:
  /// @brief initialize itsConverter
  void initConverter() const;

  /// @brief translate a name of the epoch reference frame to the type enum
  /// @details Table store the reference frame as a string and one needs a
  /// way to convert it to a enum used in the constructor of the epoch
  /// object to be able to construct it. This method provides a required
  /// translation.
  /// @param[in] name a string name of the reference frame 
  static casa::MEpoch::Types frameType(const casa::String &name);
private:
  mutable boost::shared_ptr<IEpochConverter const> itsConverter;
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef TIME_DEPENDENT_SUBTABLE_H

