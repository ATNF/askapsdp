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
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
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

using namespace askap;
using namespace askap::synthesis;


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
    ASKAPTHROW(DataAccessError, "casa::AipsError is caught inside "
         "TableTimeStampSelector::updateTableExpression: "<<ae.what());
  }
}
