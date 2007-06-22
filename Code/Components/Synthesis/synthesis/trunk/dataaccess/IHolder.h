/// @file
/// @brief A base class for classes holding something
/// @details This class is a structural item used as a base class for
/// interfaces to various data holders (i.e. table holder, derived
/// information holder, subtable data holder, etc). There is no need
/// for any methods here. It just has an empty virtual destructor to
/// avoid specifying it for a number of the next level classes.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_HOLDER_H
#define I_HOLDER_H


namespace conrad {

namespace synthesis {

/// @brief A base class for classes holding something
/// @details This class is a structural item used as a base class for
/// interfaces to various data holders (i.e. table holder, derived
/// information holder, subtable data holder, etc). There is no need
/// for any methods here. It just has an empty virtual destructor to
/// avoid specifying it for a number of the next level classes.
struct IHolder {

  /// void virtual destructor to keep the compiler happy
  virtual ~IHolder();
};

} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_TABLE_HOLDER_H
