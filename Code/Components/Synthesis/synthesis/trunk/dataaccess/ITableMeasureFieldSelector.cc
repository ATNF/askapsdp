/// @file ITableMeasureFieldSelector.cc
/// @brief Interface constraining an expression node for measure fields
/// @details The units and reference frame have to be specified via a
/// fully defined converter to form an expression node selecting a subtable
/// based on some measure-type field (e.g. time range). This interface
/// provide appropriate methods.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own includes
#include <dataaccess/ITableMeasureFieldSelector.h>

using namespace conrad;
using namespace synthesis;

/// This file contains just an empty virtual destructor
/// to keep the compiler happy
ITableMeasureFieldSelector::~ITableMeasureFieldSelector() {}
