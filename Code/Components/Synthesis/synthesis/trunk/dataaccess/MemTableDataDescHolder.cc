/// @file
/// @brief Implementation of ITableDataDescHolder holding everything in memory
/// @details This file contains a class implementing the ITableDataDescHolder
/// interface by reading the appropriate subtable into memory in the constructor.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/MemTableDataDescHolder.h>
#include <conrad/ConradError.h>

#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableRecord.h>

using namespace conrad;
using namespace conrad::synthesis;
using namespace std;
using namespace casa;


/// read all required information from the DATA_DESCRIPTION subtable
/// @param ms an input measurement set (a table which has a
/// DATA_DESCRIPTION subtable defined)
MemTableDataDescHolder::MemTableDataDescHolder(const casa::Table &ms)
{
  Table dataDescrSubtable=ms.keywordSet().asTable("DATA_DESCRIPTION");
  ROScalarColumn<Int> polID(dataDescrSubtable,"POLARIZATION_ID");
  ROScalarColumn<Int> spWinID(dataDescrSubtable,"SPECTRAL_WINDOW_ID");
  itsDataDescription.reserve(dataDescrSubtable.nrow());
  for (uInt row=0;row<dataDescrSubtable.nrow();++row) {
       itsDataDescription.push_back(pair<int,int>(spWinID(row),polID(row)));
  }
}

/// obtain spectral window ID via data description ID
/// @param dataDescriptionID an index into data description table for
///  which to return an associated spectral window ID
/// @return spectral window id for a given dataDescriptionID
/// @note return type has sign. User is responsible for interpreting
/// the negative values
int MemTableDataDescHolder::getSpectralWindowID(size_t dataDescriptionID)
                  const
{ 
  CONRADASSERT(dataDescriptionID<itsDataDescription.size());
  return itsDataDescription[dataDescriptionID].first;
}

/// obtain polaraziation ID via data description ID
/// @param dataDescriptionID an index into data description table for
///  which to return an associated polarization ID
/// @return polarization id for a given dataDescriptionID
/// @note return type has sign. User is responsible for interpreting
/// the negative values
int MemTableDataDescHolder::getPolarizationID(size_t dataDescriptionID) const
{
  CONRADASSERT(dataDescriptionID<itsDataDescription.size());
  return itsDataDescription[dataDescriptionID].second;
}

/// obtain all data description IDs which correspond to the given
/// spectral window ID (requires for selection on the spectral window)
/// @param spWindowID a spectral window ID to search for
/// @return a vector containing data description IDs
/// @note a signed type is used for spWindowID. User is responsible for
/// interpreting the negative values
std::vector<size_t>
MemTableDataDescHolder::getDescIDsForSpWinID(int spWindowID)  const
{
  vector<size_t> result;    
  for (size_t descrID=0;descrID<itsDataDescription.size();++descrID) {
       if (itsDataDescription[descrID].first==spWindowID) {
           result.push_back(descrID);
       }
  }
  return result;
}
