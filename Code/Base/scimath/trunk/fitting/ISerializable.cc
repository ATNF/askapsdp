/// @file
/// @brief An interface responsible for serialization to/from a blob stream
/// @details Methods to read from / write to a blob stream must present in
/// all classes compatible with such operation. This abstract class provides
/// interface definitions. 
/// @note It should probably go into blob, as scimath is not the appropriate
/// place for such a general interface. At this stage just declare it outside
/// the scimath namespace.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// own includes
#include <fitting/ISerializable.h>

namespace conrad {

/// @brief an empty virtual destructor to keep the compiler happy
ISerializable::~ISerializable() {}


} // namespace conrad