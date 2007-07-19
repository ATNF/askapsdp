/// @file
/// @brief A handler of  ANTENNA subtable
/// @details This class provides access to the ANTENNA subtable (which contains 
/// antenna mounts and positions for all antennas). It caches the whole table
/// in constructor and then returns cached values. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// own includes
#include <dataaccess/MemAntennaSubtableHandler.h>
#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <casa/Arrays/Array.h>

using namespace conrad;
using namespace conrad::synthesis;

/// read all required information from the ANTENNA subtable
/// @param[in] ms an input measurement set (a table which has an
/// ANTENNA subtable)
MemAntennaSubtableHandler::MemAntennaSubtableHandler(const casa::Table &ms)
{
  casa::Table antennaSubtable=ms.keywordSet().asTable("ANTENNA");
  if (!antennaSubtable.nrow()) {
      CONRADTHROW(DataAccessError, "The ANTENNA subtable is empty");      
  }
  casa::ROScalarColumn<casa::String> mountCol(antennaSubtable,"MOUNT");
  casa::ROScalarMeasColumn<casa::MPosition> posCol(antennaSubtable,"POSITION");
  mountCol.getColumn(itsMounts,casa::True);
  itsPositions.resize(itsMounts.nelements());
  casa::Vector<casa::MPosition>::iterator it=itsPositions.begin();
  for (casa::uInt ant=0; it!=itsPositions.end(); ++it,++ant) {
       *it=posCol(ant);
  }  
}
  
/// @brief obtain the position of the given antenna
/// @details
/// @param[in] antID antenna ID to return the position for
/// @return a reference to the MPosition measure
const casa::MPosition& MemAntennaSubtableHandler::getPosition(casa::uInt antID) 
                           const
{
  CONRADDEBUGASSERT(antID<itsPositions.nelements());
  return itsPositions[antID];
}                           
  
/// @brief obtain the mount type for the given antenna
/// @details
/// @param[in] antID antenna ID to return the position for
/// @return a string describing the mount type
const casa::String& MemAntennaSubtableHandler::getMount(casa::uInt antID) const
{
  CONRADDEBUGASSERT(antID<itsMounts.nelements());
  return itsMounts[antID];
}
