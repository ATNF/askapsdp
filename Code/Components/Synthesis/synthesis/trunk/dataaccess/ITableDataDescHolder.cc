/// @file
/// @brief An interface to DATA_DESCRIPTION subtable
/// @details A class derived from this interface provides access to
/// the content of the DATA_DESCRIPTION table (which connects data
/// description id with spectral window id and polarization id
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/ITableDataDescHolder.h>

using namespace conrad;
using namespace synthesis;

/// void virtual destructor to keep the compiler happy
ITableDataDescHolder::~ITableDataDescHolder() {}
