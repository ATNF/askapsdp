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

#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/TableRecord.h>

// uncomment to use the logger, if it is really used somewhere.
//#include <conrad_synthesis.h>
//#include <conrad/ConradLogging.h>
//CONRAD_LOGGER(logger, "");


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
            
  itsConverter.reset(new EpochConverter(casa::MEpoch(casa::MVEpoch(),
                     frameType(timeMeasInfo.asString("Ref"))),timeUnits));
}  


/// @brief translate a name of the epoch reference frame to the type enum
/// @details Table store the reference frame as a string and one needs a
/// way to convert it to a enum used in the constructor of the epoch
/// object to be able to construct it. This method provides a required
/// translation.
/// @param[in] name a string name of the reference frame 
casa::MEpoch::Types TimeDependentSubtable::frameType(const casa::String &name)
{
  if (name == "UTC") {
      return casa::MEpoch::UTC;
  } else if (name == "TAI" || name == "IAT") {
      return casa::MEpoch::TAI;
  } else if (name == "UT" || name == "UT1") {
      return casa::MEpoch::UT1;
  } else if (name == "UT2") {
      return casa::MEpoch::UT2;   
  } else if (name == "TDT" || name == "TT" || name == "ET") {
      return casa::MEpoch::TDT;   
  } else if (name == "GMST" || name == "GMST1") {
      return casa::MEpoch::GMST;
  } else if (name == "TCB") {
      return casa::MEpoch::TCB;   
  } else if (name == "TDB") {
      return casa::MEpoch::TDB;   
  } else if (name == "TCG") {
      return casa::MEpoch::TCG;   
  } else if (name == "LAST") {
      return casa::MEpoch::LAST;   
  } else if (name == "LMST") {
      return casa::MEpoch::LMST;   
  } else if (name == "GAST") {
      return casa::MEpoch::GAST;   
  } 
  CONRADTHROW(DataAccessError, "The frame "<<name<<
              " is not supported at the moment");
  return casa::MEpoch::UTC; // to keep the compiler happy
}
