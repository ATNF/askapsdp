/// @file
/// @brief A base class for handler of a time-dependent subtable
/// @details All classes representing time-dependent subtables are expected
/// to be derived from this one. It implements the method to 
/// convert a fully specified epoch into casa::Double intrinsically used by
/// the subtable. The actual subtable handler can use this for either 
/// an intelligent selection or efficient caching. The main idea behind this
/// class is to provide data necessary for a table
/// selection on the TIME column (which is a measure column). The class
/// reads units and the reference frame and sets up the converter.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// own includes
#include <dataaccess/TimeDependentSubtable.h>
#include <dataaccess/EpochConverter.h>
#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/TableRecord.h>


using namespace conrad;
using namespace conrad::synthesis;

/// @brief obtain time epoch in the subtable's native format
/// @details Convert a given epoch to the table's native frame/units
/// @param[in] time an epoch specified as a measure
/// @return an epoch in table's native frame/units
casa::Double TimeDependentSubtable::tableTime(const casa::MEpoch &time) const
{
  if (!itsConverter) {
      // first use, we need to read frame/unit information and set up the 
      // converter
      initConverter();
   }
  CONRADDEBUGASSERT(itsConverter);
  return (*itsConverter)(time);
}

/// @brief obtain a full epoch object for a given time (reverse conversion)
/// @details Some subtables can have more than one time-related columns, i.e.
/// TIME and INTERVAL. This method allows to form a full MEpoch measure from
/// the time represented as double in the native table's reference frame/unit.
/// It allows to extract frame/unit information and compare them with that of
/// the other columns. 
casa::MEpoch TimeDependentSubtable::tableTime(casa::Double time) const
{
  if (!itsConverter) {
      // first use, we need to read frame/unit information and set up the 
      // converter
      initConverter();
   }
  CONRADDEBUGASSERT(itsConverter);
  return itsConverter->toMeasure(time);  
}

/// @brief initialize itsConverter
void TimeDependentSubtable::initConverter() const
{
  const casa::Array<casa::String> &tabUnits=table().tableDesc().
      columnDesc("TIME").keywordSet().asArrayString("QuantumUnits");
  if (tabUnits.nelements()!=1 || tabUnits.ndim()!=1) {
      CONRADTHROW(DataAccessError, "Unable to interpret the QuantumUnits "
        "keyword for the TIME column of a time-dependent subtable (type="<<
         table().tableInfo().type()<<"). It should be a 1D Array of exactly "
        "one String element and the table has "<<tabUnits.nelements()<<
        " elements and "<<tabUnits.ndim()<<" dimensions");
  }
  const casa::Unit timeUnits=casa::Unit(tabUnits(casa::IPosition(1,0)));
  
  const casa::RecordInterface &timeMeasInfo=table().tableDesc().
        columnDesc("TIME").keywordSet().asRecord("MEASINFO");
  CONRADASSERT(timeMeasInfo.asString("type")=="epoch");
      
  // to implement other frames than UTC we need here a conversion
  // between the string and enum allowed by MEpoch::Ref.
  // this is left aside because there is no a use case for this
  // functionality in Conrad.
  if (timeMeasInfo.asString("Ref")!="UTC") {
      CONRADTHROW(DataAccessError, "The frame "<<timeMeasInfo.asString("Ref")<<
       " is not supported, only UTC is supported");
  }
      
  itsConverter.reset(new EpochConverter(casa::MEpoch(),timeUnits));
}  
