/// @file
/// @brief A handler of  ANTENNA subtable
/// @details This class provides access to the ANTENNA subtable (which contains 
/// antenna mounts and positions for all antennas). It caches the whole table
/// in constructor and then returns cached values. 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// own includes
#include <dataaccess/MemAntennaSubtableHandler.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <casa/Arrays/Array.h>

using namespace askap;
using namespace askap::synthesis;

/// read all required information from the ANTENNA subtable
/// @param[in] ms an input measurement set (a table which has an
/// ANTENNA subtable)
MemAntennaSubtableHandler::MemAntennaSubtableHandler(const casa::Table &ms) : 
       itsAllEquatorial(true)
{
  casa::Table antennaSubtable=ms.keywordSet().asTable("ANTENNA");
  if (!antennaSubtable.nrow()) {
      ASKAPTHROW(DataAccessError, "The ANTENNA subtable is empty");      
  }
  casa::ROScalarColumn<casa::String> mountCol(antennaSubtable,"MOUNT");
  casa::ROScalarMeasColumn<casa::MPosition> posCol(antennaSubtable,"POSITION");
  mountCol.getColumn(itsMounts,casa::True);
  itsPositions.resize(itsMounts.nelements());
  casa::Vector<casa::MPosition>::iterator it=itsPositions.begin();
  casa::Vector<casa::String>::const_iterator cit=itsMounts.begin();
  for (casa::uInt ant=0; it!=itsPositions.end(); ++it,++ant,++cit) {
       *it=posCol(ant);
       const casa::String &cMount = *cit;
       if (cMount != "EQUATORIAL" && cMount != "equatorial") {
           itsAllEquatorial = false;
       }
  }  
}
  
/// @brief obtain the position of the given antenna
/// @details
/// @param[in] antID antenna ID to return the position for
/// @return a reference to the MPosition measure
const casa::MPosition& MemAntennaSubtableHandler::getPosition(casa::uInt antID) 
                           const
{
  ASKAPDEBUGASSERT(antID<itsPositions.nelements());
  return itsPositions[antID];
}                           
  
/// @brief obtain the mount type for the given antenna
/// @details
/// @param[in] antID antenna ID to return the position for
/// @return a string describing the mount type
const casa::String& MemAntennaSubtableHandler::getMount(casa::uInt antID) const
{
  ASKAPDEBUGASSERT(antID<itsMounts.nelements());
  return itsMounts[antID];
}

/// @brief check whether all antennae are equatorialy mounted
/// @details
/// This method checks the mount type for all antennas to be 
/// either EQUATORIAL or equatorial. This mount type doesn't require
/// parallactic angle rotation and can be trated separately.
/// @return true, if all antennae are equatorially mounted
bool MemAntennaSubtableHandler::allEquatorial() const throw()
{  
  return itsAllEquatorial;
}   
