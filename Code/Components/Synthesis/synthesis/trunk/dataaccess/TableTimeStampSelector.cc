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

// own includes
#include <dataaccess/TableTimeStampSelector.h>
#include <dataaccess/DataAccessError.h>

using namespace conrad;
using namespace conrad::synthesis;

/// constructor, stores the table which will later be used to form
/// expression nodes
/// @param[in] tab a measurement set
TableTimeStampSelector::TableTimeStampSelector(const casa::Table &tab) :
         itsMS(tab) {}


/// main method, updates table expression node to narrow down the selection
///
/// @param tex a reference to table expression to use
void TableTimeStampSelector::updateTableExpression(casa::TableExprNode &tex)
	                                           const
{
  try {    
    const casa::TableRecord &timeColKeywords =
            itsMS.tableDesc()["TIME"].keywordSet();
    const casa::Array<casa::String> &measInfo =
            timeColKeywords.asArrayString("MEASINFO");
    CONRADASSERT(measInfo.nelements()>=2);
    CONRADASSERT(measInfo.ndim()==1);
    CONRADASSERT(measInfo(casa::IPosition(1,0))=="epoch");
    std::pair<casa::Double, casa::Double>
            startAndStop = getStartAndStop(measInfo(casa::IPosition(1,1)),
                                    timeColKeywords.asString("QuantumUnits"));
    if (tex.isNull()) {
        tex=(itsMS.col("TIME") > startAndStop.first) &&
            (itsMS.col("TIME") < startAndStop.second);
    } else {
        tex=tex && (itsMS.col("TIME") > startAndStop.first) &&
                   (itsMS.col("TIME") < startAndStop.second);
    }
  }
  catch(const casa::AipsError &ae) {
    CONRADTHROW(DataAccessError, "casa::AipsError is caught inside "
         "TableTimeStampSelector::updateTableExpression: "<<ae.what());
  }
}
