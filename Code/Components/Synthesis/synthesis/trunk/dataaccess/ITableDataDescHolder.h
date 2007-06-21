/// @file
/// @brief An interface to DATA_DESCRIPTION subtable
/// @details A class derived from this interface provides access to
/// the content of the DATA_DESCRIPTION table (which connects data
/// description id with spectral window id and polarization id
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_DATA_DESC_HOLDER_H
#define I_TABLE_DATA_DESC_HOLDER_H

// std includes
#include <vector>

// own includes
#include <dataaccess/ITableHolder.h>

namespace conrad {

namespace synthesis {

/// @brief An interface to DATA_DESCRIPTION subtable
/// @details A class derived from this interface provides access to
/// the content of the DATA_DESCRIPTION table (which connects data
/// description id with spectral window id and polarization id
struct ITableDataDescHolder {

  /// void virtual destructor to keep the compiler happy
  virtual ~ITableDataDescHolder();

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

} // namespace conrad

#endif // #ifndef I_TABLE_DATA_DESC_HOLDER_H
