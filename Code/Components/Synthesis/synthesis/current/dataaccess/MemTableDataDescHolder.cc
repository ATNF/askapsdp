/// @file
/// @brief Implementation of ITableDataDescHolder holding everything in memory
/// @details This file contains a class implementing the ITableDataDescHolder
/// interface by reading the appropriate subtable into memory in the constructor.
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

#include <dataaccess/MemTableDataDescHolder.h>
#include <askap_synthesis.h>

// enable logging when it is actually used
//#include <askap/AskapLogging.h>
//ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>

#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableRecord.h>

using namespace askap;
using namespace askap::synthesis;
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
  ASKAPASSERT(dataDescriptionID<itsDataDescription.size());
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
  ASKAPASSERT(dataDescriptionID<itsDataDescription.size());
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
