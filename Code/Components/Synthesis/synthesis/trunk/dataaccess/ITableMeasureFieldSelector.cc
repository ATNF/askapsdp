/// @file ITableMeasureFieldSelector.h
///
/// ITableMeasureFieldSelector: an interface to constrain a table selection
///                     object (expression node) for a field which is
///                     a measure (i.e. requires a fully defined converter
///                     to complete processing)
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
