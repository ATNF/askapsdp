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
#include <dataaccess/TableFieldHolder.h>
#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScalarColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>


using namespace conrad;
using namespace conrad::synthesis;

/// @brief construct the object
/// @details 
/// @param[in] ms a table object, which has a field subtable defined
/// (i.e. this method accepts a main ms table).
TableFieldHolder::TableFieldHolder(const casa::Table &ms) :
       itsFieldSubtable(ms.keywordSet().asTable("FIELD")),
       itsIterator(itsFieldSubtable,"TIME",casa::TableIterator::DontCare,
                   casa::TableIterator::NoSort)
{
  const casa::Array<casa::String> &tabUnits=itsFieldSubtable.tableDesc().
        columnDesc("TIME").keywordSet().asArrayString("QuantumUnits");
  if (tabUnits.nelements()!=1 || tabUnits.ndim()!=1) {
      CONRADTHROW(DataAccessError, "Unable to interpret the QuantumUnits "
        "keyword for the TIME column of the FIELD subtable. It should be a "
        "1D Array of exactly 1 String element and the table has "<<tabUnits.nelements()<<
        " elements and "<<tabUnits.ndim()<<" dimensions");
  }
  itsTimeUnits=casa::Unit(tabUnits(casa::IPosition(1,0)));
  
  const casa::RecordInterface &timeMeasInfo=itsFieldSubtable.tableDesc().
            columnDesc("TIME").keywordSet().asRecord("MEASINFO");
  CONRADASSERT(timeMeasInfo.asString("type")=="epoch");
  if (timeMeasInfo.asString("Ref")!="UTC") {
      CONRADTHROW(DataAccessError, "The frame "<<timeMeasInfo.asString("Ref")<<
           " is not supported, only UTC is supported");
  }
  if (!itsFieldSubtable.nrow()) {
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
const casa::MDirection& TableFieldHolder::getReferenceDir(const 
                 casa::MEpoch &time) const
{
  fillCacheOnDemand(time);
  return itsReferenceDir;
}                 

/// read the current iteration and populate cache. It also advances the
/// iterator
void TableFieldHolder::fillCacheWithCurrentIteration() const
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
  if (!itsIterator.pastEnd()) {
      itsIterator.next();
      itsCachedStopTime=timeCol(1);
  }
}

/// read the data if cache is outdated
/// @param[in] time a full epoch of interest (field table can have many
/// pointing and therefore can be time-dependent)
void TableFieldHolder::fillCacheOnDemand(const casa::MEpoch &time) const
{
  CONRADASSERT(time.getRef().getType() == casa::MEpoch::UTC);
  casa::Double dTime=time.getValue().getTime(itsTimeUnits).getValue();
  if (dTime<itsCachedStartTime) {
      itsIterator.reset();
      fillCacheWithCurrentIteration();
  } 
  if (dTime<itsCachedStartTime) {
      CONRADTHROW(DataAccessError, "An earlier time is requested ("<<time<<") than "
             "the FIELD table has data for");
  }
  if ((itsFieldSubtable.nrow() == 1) || 
      (dTime>=itsCachedStartTime && dTime<=itsCachedStopTime)) {      
      return;
  }
  while (!itsIterator.pastEnd() && ((dTime<itsCachedStartTime) || 
         (dTime>itsCachedStopTime))) {
     fillCacheWithCurrentIteration(); 
  }
  CONRADDEBUGASSERT(dTime>=itsCachedStartTime);
}
