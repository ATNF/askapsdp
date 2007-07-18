/// @file
/// @brief An interface to FIELD subtable
/// @details A class derived from this interface provides access to
/// the content of the FIELD subtable (which provides delay, phase and
/// reference centres for each time). The POINTING table gives the actual 
/// pointing of the antennae. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_FIELD_HOLDER_H
#define I_TABLE_FIELD_HOLDER_H

// casa includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>

// own includes
#include <dataaccess/IHolder.h>

namespace conrad {

namespace synthesis {

/// @brief An interface to FIELD subtable
/// @details A class derived from this interface provides access to
/// the content of the FIELD subtable (which provides delay, phase and
/// reference centres for each time). The POINTING table gives the actual 
/// pointing of the antennae. 
/// @ingroup dataaccess_tab
struct ITableFieldHolder : virtual public IHolder {
  
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
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_TABLE_FIELD_HOLDER_H
