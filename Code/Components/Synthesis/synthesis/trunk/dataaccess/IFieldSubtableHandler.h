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

#ifndef I_FIELD_SUBTABLE_HANLDER_H
#define I_FIELD_SUBTABLE_HANLDER_H

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
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_FIELD_SUBTABLE_HANLDER_H
