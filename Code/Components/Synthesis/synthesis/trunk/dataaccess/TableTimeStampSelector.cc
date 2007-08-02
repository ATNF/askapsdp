/// @file TableTimeStampSelector.cc
/// @brief Generalized selection on epoch in the table-based case
/// @details This class representing a selection of visibility
///          data on some time interval. It implements the abstract
///          method updating table expression node via a new
///          abstract method, which just return start and stop
///          times as Double in the same frame/units as the TIME
///          column in the table. These two methods are specified
///          in the derived classes.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// casa includes
#include <casa/Exceptions/Error.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ColumnDesc.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MeasConvert.h>

// own includes
#include <dataaccess/TableTimeStampSelector.h>
#include <dataaccess/DataAccessError.h>

using namespace conrad;
using namespace conrad::synthesis;


/// main method, updates table expression node to narrow down the selection
///
/// @param tex a reference to table expression to use
void TableTimeStampSelector::updateTableExpression(casa::TableExprNode &tex)
	                                           const
{
  try {    
    const std::pair<casa::MEpoch, casa::MEpoch> startAndStop = getStartAndStop();
    
    const casa::Double start = tableTime(startAndStop.first);
    const casa::Double stop = tableTime(startAndStop.second);
            
    if (tex.isNull()) {
        tex=(table().col("TIME") > start) &&
            (table().col("TIME") < stop);
    } else {
        tex=tex && (table().col("TIME") > start) &&
                   (table().col("TIME") < stop);
    }
  }
  catch(const casa::AipsError &ae) {
    CONRADTHROW(DataAccessError, "casa::AipsError is caught inside "
         "TableTimeStampSelector::updateTableExpression: "<<ae.what());
  }
}
