/// @file
/// @brief An interface to DATA_DESCRIPTION subtable
/// @details A class derived from this interface provides access to
/// the content of the DATA_DESCRIPTION table (which connects data
/// description id with spectral window id and polarization id
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

#ifndef I_TABLE_DATA_DESC_HOLDER_H
#define I_TABLE_DATA_DESC_HOLDER_H

// std includes
#include <vector>

// own includes
#include <dataaccess/IHolder.h>

namespace askap {

namespace synthesis {

/// @brief An interface to DATA_DESCRIPTION subtable
/// @details A class derived from this interface provides access to
/// the content of the DATA_DESCRIPTION table (which connects data
/// description id with spectral window id and polarization id
/// @ingroup dataaccess_tab
struct ITableDataDescHolder : virtual public IHolder {

  /// obtain spectral window ID via data description ID
  /// @param dataDescriptionID an index into data description table for
  ///  which to return an associated spectral window ID
  /// @return spectral window id for a given dataDescriptionID
  /// @note return type has sign. User is responsible for interpreting
  /// the negative values
  virtual int getSpectralWindowID(size_t dataDescriptionID) const = 0;

  /// obtain polaraziation ID via data description ID
  /// @param dataDescriptionID an index into data description table for
  ///  which to return an associated polarization ID
  /// @return polarization id for a given dataDescriptionID
  /// @note return type has sign. User is responsible for interpreting
  /// the negative values
  virtual int getPolarizationID(size_t dataDescriptionID) const = 0;

  /// obtain all data description IDs which correspond to the given
  /// spectral window ID (requires for selection on the spectral window)
  /// @param spWindowID a spectral window ID to search for
  /// @return a vector containing data description IDs
  /// @note a signed type is used for spWindowID. User is responsible for
  /// interpreting the negative values
  virtual std::vector<size_t> getDescIDsForSpWinID(int spWindowID) const = 0;  
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef I_TABLE_DATA_DESC_HOLDER_H
