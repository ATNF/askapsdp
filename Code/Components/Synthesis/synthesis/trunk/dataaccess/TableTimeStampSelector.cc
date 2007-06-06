/// @file TableTimeStampSelector.cc
///
/// TableTimeStampSelector: Class representing a selection of visibility
///                data on some time interval. It implements the abstract
///                method updating table expression node via two new
///                abstract methods, which just return start and stop
///                times as Double in the same frame/units as the TIME
///                column in the table. These two methods are specified
///                in the derived classes.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// own includes
#include <dataaccess/TableTimeStampSelector.h>

using namespace conrad;
using namespace synthesis;

/// main method, updates table expression node to narrow down the selection
///
/// @param tex a reference to table expression to use
void TableTimeStampSelector::updateTableExpression(casa::TableExprNode &tex)
{
  tex=tex && (tex.table().col("TIME") > start()) &&
             (tex.table().col("TIME") < stop());
}
