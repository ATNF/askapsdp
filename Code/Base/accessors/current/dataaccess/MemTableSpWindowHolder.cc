/// @file
/// @brief Implementation of ITableSpWindowHolder
/// @details This file contains a class, which reads and stores 
/// the content of the SPECTRAL_WINDOW subtable (which provides
/// frequencies for each channel). The table is indexed with the
/// spectral window ID.
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

// own includes
#include <dataaccess/MemTableSpWindowHolder.h>

//enable logging when it is actually used
//#include <askap_accessors.h>
//#include <askap/AskapLogging.h>
//ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVFrequency.h>


using namespace askap;
using namespace askap::accessors;
using namespace casa;

/// read all required information from the SPECTRAL_WINDOW subtable
/// @param ms an input measurement set (in fact any table which has a
/// SPECTRAL_WINDOW subtable defined)
MemTableSpWindowHolder::MemTableSpWindowHolder(const casa::Table &ms)
{
  Table spWindowSubtable=ms.keywordSet().asTable("SPECTRAL_WINDOW");

  // load units 
  const Array<String> &tabUnits=spWindowSubtable.tableDesc().
          columnDesc("CHAN_FREQ").keywordSet().asArrayString("QuantumUnits");
  if (tabUnits.nelements()!=1 || tabUnits.ndim()!=1) {
      ASKAPTHROW(DataAccessError,"Unable to interpret the QuantumUnits keyword "<<
                  "for the CHAN_FREQ column of the SPECTRAL_WINDOW subtable. "<<
		  "It should be an 1D Array of 1 String element and it has "<<
		  tabUnits.nelements()<<" elements and "<<tabUnits.ndim()<<
		  " dimensions");
  }  
  itsFreqUnits=casa::Unit(tabUnits(IPosition(1,0)));
  
  // load reference frame ids
  ROScalarColumn<Int> measRefCol(spWindowSubtable,"MEAS_FREQ_REF");
  measRefCol.getColumn(itsMeasRefIDs,True);
  
  // load channel frequencies
  ROArrayColumn<Double> chanFreqCol(spWindowSubtable,"CHAN_FREQ");
  ASKAPDEBUGASSERT(measRefCol.nrow()==chanFreqCol.nrow());
  itsChanFreqs.resize(spWindowSubtable.nrow());
  for (uInt row=0;row<spWindowSubtable.nrow();++row) {
       ASKAPASSERT(chanFreqCol.ndim(row)==1);
       chanFreqCol.get(row,itsChanFreqs[row]); 
  }  
}

/// obtain the reference frame used in the spectral window table
/// @param[in] spWindowID an index into spectral window table
/// @return the reference frame of the given row
casa::MFrequency::Ref
    MemTableSpWindowHolder::getReferenceFrame(casa::uInt spWindowID) const
{
 ASKAPDEBUGASSERT(spWindowID<itsMeasRefIDs.nelements());
 return MFrequency::Ref(itsMeasRefIDs[spWindowID]);
}

/// @brief obtain the frequency units used in the spectral window table
/// @details The frequency units depend on the measurement set only and
/// are the same for all rows.
/// @return a reference to the casa::Unit object
const casa::Unit& MemTableSpWindowHolder::getFrequencyUnit() const throw()
{
  return itsFreqUnits;
}

/// @brief obtain frequencies for each spectral channel
/// @details All frequencies for each spectral channel are retreived as
/// Doubles at once. The units and reference frame can be obtained
/// via getReferenceFrame and getFrequencyUnit methods of this class.  
/// @param[in] spWindowID an index into spectral window table
/// @return freqs a const reference to a vector with result
const casa::Vector<casa::Double>&
MemTableSpWindowHolder::getFrequencies(casa::uInt spWindowID) const
{
  ASKAPDEBUGASSERT(spWindowID<itsChanFreqs.nelements());
  return itsChanFreqs[spWindowID];
}

/// @brief obtain frequency for a given spectral channel
/// @details This version of the method is intended to obtain a
/// frequency of a given spectral channel as fully qualified measure.
/// The intention is to use this method if the conversion is required
/// (and, hence, element by element operations are needed anyway)
/// @param[in] spWindowID an index into spectral window table
/// @param[in] channel a channel number of interest
casa::MFrequency MemTableSpWindowHolder::getFrequency(casa::uInt spWindowID,
                          casa::uInt channel) const
{
  ASKAPDEBUGASSERT(spWindowID<itsChanFreqs.nelements());  
  ASKAPDEBUGASSERT(channel<itsChanFreqs[spWindowID].nelements());
  const casa::Double freqAsDouble=itsChanFreqs[spWindowID][channel];
  const casa::MVFrequency result(Quantity(freqAsDouble,itsFreqUnits));
  return MFrequency(result,MFrequency::Ref(itsMeasRefIDs[spWindowID]));
}
