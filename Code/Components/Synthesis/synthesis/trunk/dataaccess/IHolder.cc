/// @file
/// @brief A base class for classes holding something
/// @details This class is a structural item used as a base class for
/// interfaces to various data holders (i.e. table holder, derived
/// information holder, subtable data holder, etc). There is no need
/// for any methods here. It just has an empty virtual destructor to
/// avoid specifying it for a number of the next level classes.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/IHolder.h>

namespace askap {

namespace synthesis {

/// void virtual destructor to keep the compiler happy
IHolder::~IHolder()
{
}

} // namespace synthesis

} // namespace askap
