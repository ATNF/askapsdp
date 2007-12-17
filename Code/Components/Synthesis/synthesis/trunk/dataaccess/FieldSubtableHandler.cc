/// @file
/// @brief A handler of the FIELD subtable
/// @details This class derived provides access to
/// the content of the FIELD subtable (which provides delay, phase and
/// reference centres for each time). The POINTING table gives the actual 
/// pointing of the antennae. Although this implementation caches the values
/// for the last requested time range, it reads the data on-demand. This is 
/// a difference from subtable handler classes, whose name starts from Mem...
/// The latter classes read all the subtable into memory in the constructor and
/// later return cached values. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// own includes
#include <dataaccess/FieldSubtableHandler.h>
#include <conrad_synthesis.h>

#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScalarColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>

// uncomment logger when it is actually used
//#include <conrad/ConradLogging.h>
//CONRAD_LOGGER(logger, "");


using namespace conrad;
using namespace conrad::synthesis;

/// @brief construct the object
/// @details 
/// @param[in] ms a table object, which has a field subtable defined
/// (i.e. this method accepts a main ms table).
FieldSubtableHandler::FieldSubtableHandler(const casa::Table &ms) :
       TableHolder(ms.keywordSet().asTable("FIELD")),
       itsIterator(table(),"TIME",casa::TableIterator::DontCare,
                   casa::TableIterator::NoSort), itsNeverAccessedFlag(true)
{
  if (!table().nrow()) {
      CONRADTHROW(DataAccessError, "The FIELD subtable is empty");
  }  
  fillCacheWithCurrentIteration();  
}         
  
/// @brief obtain the reference direction for a given time.
/// @details It is not clear at the moment whether this subtable is
/// useful in the multi-beam case because each physical feed corresponds to
/// its own phase- and delay tracking centre. It is assumed at the moment
/// that the reference direction can be used as the dish pointing direction
/// in the absence of the POINTING subtable. It is not clear what this
/// direction should be in the case of scanning.
/// @param[in] time a full epoch of interest (the subtable can have multiple
/// pointings.
/// @return a reference to direction measure
const casa::MDirection& FieldSubtableHandler::getReferenceDir(const 
                 casa::MEpoch &time) const
{
  fillCacheOnDemand(time);
  itsNeverAccessedFlag=false;
  return itsReferenceDir;
}                 

/// read the current iteration and populate cache. It also advances the
/// iterator
void FieldSubtableHandler::fillCacheWithCurrentIteration() const
{
  casa::Table curIt=itsIterator.table(); 
  if (curIt.nrow()>1) {
      CONRADTHROW(DataAccessError, "Multiple rows for the same TIME in the FIELD table "
          "(e.g. polynomial interpolation) are not yet supported");
  }
  casa::ROScalarColumn<casa::Double> timeCol(curIt,"TIME");
  itsCachedStartTime=timeCol(0);
  casa::ROScalarMeasColumn<casa::MDirection> refDirCol(curIt,"REFERENCE_DIR");
  itsReferenceDir=refDirCol(0);
  CONRADDEBUGASSERT(!itsIterator.pastEnd());
  itsIterator.next();
  if (!itsIterator.pastEnd()) {
      itsCachedStopTime=timeCol(0);
  }    
}

/// @brief check whether the field changed for a given time
/// @details The users of this class can do relatively heavy calculations
/// depending on the field position on the sky. It is, therefore, practical
/// to assist caching by providing a method to test whether the cache is
/// still valid or not for a new time. Use this method instead of testing
/// whether directions are close enough as it can make use the information
/// stored in the subtable. The method always returns true before the 
/// first access to the data.
/// @param[in] time a full epoch of interest (the subtable can have multiple
/// pointings.
/// @return true if the field information have been changed
bool FieldSubtableHandler::newField(const casa::MEpoch &time) const
{
  if (itsNeverAccessedFlag) {
      return true;
  }
  // we may need caching of dTime if it becomes performance critical
  const casa::Double dTime=tableTime(time);
  if (dTime<itsCachedStartTime) {
      return true;
  }
  if (table().nrow() == 1) {
      return false;
  }
  return (dTime>itsCachedStopTime);
}

/// read the data if cache is outdated
/// @param[in] time a full epoch of interest (field table can have many
/// pointings and therefore can be time-dependent)
void FieldSubtableHandler::fillCacheOnDemand(const casa::MEpoch &time) const
{
  const casa::Double dTime=tableTime(time);
  if (dTime<itsCachedStartTime) {
      itsIterator.reset();
      fillCacheWithCurrentIteration();
  } 
  if (dTime<itsCachedStartTime) {
      CONRADTHROW(DataAccessError, "An earlier time is requested ("<<time<<") than "
             "the FIELD table has data for");
  }
  if ((table().nrow() == 1) || 
      (dTime>=itsCachedStartTime && dTime<=itsCachedStopTime)) {      
      return;
  }
  while (!itsIterator.pastEnd() && ((dTime<itsCachedStartTime) || 
         (dTime>itsCachedStopTime))) {
     fillCacheWithCurrentIteration(); 
  }
  CONRADDEBUGASSERT(dTime>=itsCachedStartTime);
}
