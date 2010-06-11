/// @file
/// @brief An interface to FIELD subtable
/// @details A class derived from this interface provides access to
/// the content of the FIELD subtable (which provides delay, phase and
/// reference centres for each time). The POINTING table gives the actual 
/// pointing of the antennae. 
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

#ifndef I_FIELD_SUBTABLE_HANLDER_H
#define I_FIELD_SUBTABLE_HANLDER_H

// casa includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>

// own includes
#include <dataaccess/IHolder.h>

namespace askap {

namespace synthesis {

/// @brief An interface to FIELD subtable
/// @details A class derived from this interface provides access to
/// the content of the FIELD subtable (which provides delay, phase and
/// reference centres for each time). The POINTING table gives the actual 
/// pointing of the antennae. 
/// @ingroup dataaccess_tab
struct IFieldSubtableHandler : virtual public IHolder {
  
  /// @brief obtain the reference direction for a given time.
  /// @details It is not clear at the moment whether this subtable is
  /// useful in the multi-beam case because each physical feed corresponds to
  /// its own phase- and delay tracking centre. It is assumed at the moment
  /// that the reference direction can be used as the dish pointing direction
  /// in the absence of the POINTING subtable. It is not clear what this
  /// direction should be in the case of scanning.
  /// @param[in] time a full epoch of interest (the subtable can have multiple
  /// pointings.
  /// @return a reference to direction measure
  virtual const casa::MDirection& getReferenceDir(const casa::MEpoch &time) 
                                                  const = 0;

  /// @brief check whether the field changed for a given time
  /// @details The users of this class can do relatively heavy calculations
  /// depending on the field position on the sky. It is, therefore, practical
  /// to assist caching by providing a method to test whether the cache is
  /// still valid or not for a new time. Use this method instead of testing
  /// whether directions are close enough as it can make use the information
  /// stored in the subtable. The method always returns true before the 
  /// first access to the data.
  /// @param[in] time a full epoch of interest (the subtable can have multiple
  /// pointings.
  /// @return true if the field information have been changed
  virtual bool newField(const casa::MEpoch &time) const = 0;

  /// @brief obtain the reference direction stored in a given row
  /// @details The measurement set format looks a bit redundant: individual
  /// pointings can be discriminated by time of observations or by a
  /// FIELD_ID. The latter is interpreted as a row number in the FIELD
  /// table and can be used for a quick access to the direction information.
  /// For ASKAP we will probably end up using just time, but the measurement
  /// sets with real data (e.g. converted from fits) all have FIELD_ID column.
  /// For simple measurement sets either method works fine. However, the
  /// discrimination by time breaks for ATCA mosaicing datasets. This method
  /// allows to avoid this problem. The current code uses FIELD_ID if
  /// it is present in the main table of the dataset.
  /// @param[in] fieldID  a row number of interest
  /// @return a reference to direction measure
  virtual const casa::MDirection& getReferenceDir(casa::uInt fieldID) 
                                                  const = 0;
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef I_FIELD_SUBTABLE_HANLDER_H
