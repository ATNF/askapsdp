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

#ifndef I_TIME_DEPENDENT_SUBTABLE_H
#define I_TIME_DEPENDENT_SUBTABLE_H

// casa includes
#include <measures/Measures/MEpoch.h>

// own includes
#include <dataaccess/ITableHolder.h>

namespace askap {

namespace accessors {


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
  
  /// @brief obtain a full epoch object for a given time (reverse conversion)
  /// @details Some subtables can have more than one time-related columns, i.e.
  /// TIME and INTERVAL. This method allows to form a full MEpoch measure from
  /// the time represented as double in the native table's reference frame/unit.
  /// It allows to extract frame/unit information and compare them with that of
  /// the other columns. 
  /// @param[in] time time to translate into a full epoch
  /// @return[in] full epoch corresponding to a given time
  virtual casa::MEpoch tableTime(casa::Double time) const = 0;
};


} // namespace accessors

} // namespace askap

#endif // #ifndef I_TIME_DEPENDENT_SUBTABLE_H

