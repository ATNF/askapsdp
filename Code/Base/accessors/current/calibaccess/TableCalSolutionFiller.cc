/// @file
/// @brief solution filler reading required cubes from casa table
/// @details This is an example of a class which knows how to fill buffers 
/// of MemCalSolutionAccessor. The cubes with calibration information are read 
/// from (and written to) a casa table. The table has the following columns:
/// TIME, GAIN, GAIN_VALID, LEAKAGE, LEAKAGE_VALID, BANDPASS and BANDPASS_VALID.
/// This class is initialised with the reference row, which corresponds to the time
/// requested by the user. If there are gains, leakages or bandpasses defined for 
/// a given row, they are read. Otherwise, a backward search is performed to find
/// the first defined value. An exception is thrown if the top of the table is reached.
/// If a new entry needs to be created, the given numbers of antennas and beams are used.
/// 
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#include <calibaccess/TableCalSolutionFiller.h>

namespace askap {

namespace accessors {

/// @brief helper method to check whether we are creating a new row
void TableCalSolutionFiller::checkForNewRow()
{
  ASKAPDEBUGASSERT(itsRefRow <= long(table().nrow()));
  itsCreateNew = (itsRefRow + 1 == long(table().nrow()));
  if (itsCreateNew) {
      const bool gainsWritten = cellDefined<casa::Complex>("GAIN", casa::uInt(itsRefRow));
      const bool leakagesWritten = cellDefined<casa::Complex>("LEAKAGE", casa::uInt(itsRefRow));
      const bool bpWritten = cellDefined<casa::Complex>("BANDPASS", casa::uInt(itsRefRow));
      itsCreateNew = !gainsWritten && !leakagesWritten && !bpWritten;
  }
  if (itsCreateNew) {
      // this is a new row in the table to be created, only TIME column exists
      ASKAPCHECK(itsNAnt > 0, "TableCalSolutionFiller needs to know the number of antennas to be able to setup new table rows");
      ASKAPCHECK(itsNBeam > 0, "TableCalSolutionFiller needs to know the number of beams to be able to setup new table rows");
      ASKAPCHECK(itsNChan > 0, "TableCalSolutionFiller needs to know the number of spectral channels to be able to setup new table rows");      
  }
}

/// @brief construct the object and link it to the given table
/// @details read-only operation is assumed
/// @param[in] tab  table to use
/// @param[in] row reference row
TableCalSolutionFiller::TableCalSolutionFiller(const casa::Table& tab, const long row) : TableHolder(tab), 
       TableBufferManager(tab), itsNAnt(0), itsNBeam(0), itsNChan(0), itsRefRow(row), itsGainsRow(-1),
       itsLeakagesRow(-1), itsBandpassesRow(-1), itsCreateNew(false)
{
  ASKAPDEBUGASSERT(row>=0);
  checkForNewRow();
}

/// @brief construct the object and link it to the given table
/// @details Maximum allowed numbers of antennas, beams and spectral channels are
/// set by this constructor which is essential for read-write operations (i.e. new
/// table entries may need to be created
/// @param[in] tab  table to use
/// @param[in] row reference row
/// @param[in] nAnt maximum number of antennas
/// @param[in] nBeam maximum number of beams   
/// @param[in] nChan maximum number of channels   
TableCalSolutionFiller::TableCalSolutionFiller(const casa::Table& tab, const long row, const casa::uInt nAnt, 
          const casa::uInt nBeam, const casa::uInt nChan) : TableHolder(tab), 
       TableBufferManager(tab), itsNAnt(nAnt), itsNBeam(nBeam), itsNChan(nChan), itsRefRow(row), itsGainsRow(-1),
       itsLeakagesRow(-1), itsBandpassesRow(-1), itsCreateNew(false) 
{
  ASKAPDEBUGASSERT(row>=0);
  checkForNewRow();
}

/// @brief helper method to check that the given column exists
/// @param[in] name column name
/// @return true if the given column exists
bool TableCalSolutionFiller::columnExists(const std::string &name) const
{
  return table().actualTableDesc().isColumn(name);
}

/// @brief check for gain solution
/// @return true, if there is no gain solution, false otherwise
bool TableCalSolutionFiller::noGain() const
{
  return !columnExists("GAIN");
}
  
/// @brief check for leakage solution
/// @return true, if there is no leakage solution, false otherwise
bool TableCalSolutionFiller::noLeakage() const
{
  return !columnExists("LEAKAGE");
}
  
/// @brief check for bandpass solution
/// @return true, if there is no bandpass solution, false otherwise
bool TableCalSolutionFiller::noBandpass() const
{
  return !columnExists("BANDPASS");
}


/// @brief gains filler  
/// @details
/// @param[in] gains pair of cubes with gains and validity flags (to be resised to 2 x nAnt x nBeam)
void TableCalSolutionFiller::fillGains(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &gains) const
{
  if (itsCreateNew || noGain()) {
      ASKAPDEBUGASSERT(itsGainsRow < 0);
      gains.first.resize(2, itsNAnt, itsNBeam);
      gains.first.set(1.);
      gains.second.resize(2, itsNAnt, itsNBeam);
      gains.second.set(false);
      itsGainsRow = itsRefRow;      
  } else {  
     if (itsGainsRow < 0) {
         itsGainsRow = findDefinedCube("GAIN");
     }
     ASKAPASSERT(itsGainsRow>=0);
     ASKAPCHECK(cellDefined<casa::Bool>("GAIN_VALID", casa::uInt(itsGainsRow)), 
         "Wrong format of the calibration table: GAIN element should always be accompanied by GAIN_VALID");
     readCube(gains.first, "GAIN", casa::uInt(itsGainsRow));
     readCube(gains.second, "GAIN_VALID", casa::uInt(itsGainsRow));
  }
  ASKAPCHECK(gains.first.shape() == gains.second.shape(), "GAIN and GAIN_VALID cubes are expected to have the same shape");
}
  
/// @brief leakage filler  
/// @details
/// @param[in] leakages pair of cubes with leakages and validity flags (to be resised to 2 x nAnt x nBeam)
void TableCalSolutionFiller::fillLeakages(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &leakages) const
{
  if (itsCreateNew || noLeakage()) {
      ASKAPDEBUGASSERT(itsLeakagesRow < 0);
      leakages.first.resize(2, itsNAnt, itsNBeam);
      leakages.first.set(0.);
      leakages.second.resize(2, itsNAnt, itsNBeam);
      leakages.second.set(false);
      itsLeakagesRow = itsRefRow;      
  } else {  
     if (itsLeakagesRow < 0) {
         itsLeakagesRow = findDefinedCube("LEAKAGE");
     }
     ASKAPASSERT(itsLeakagesRow>=0);
     ASKAPCHECK(cellDefined<casa::Bool>("LEAKAGE_VALID", casa::uInt(itsLeakagesRow)), 
         "Wrong format of the calibration table: LEAKAGE element should always be accompanied by LEAKAGE_VALID");
     readCube(leakages.first, "LEAKAGE", casa::uInt(itsLeakagesRow));
     readCube(leakages.second, "LEAKAGE_VALID", casa::uInt(itsLeakagesRow));
  }
  ASKAPCHECK(leakages.first.shape() == leakages.second.shape(), "LEAKAGE and LEAKAGE_VALID cubes are expected to have the same shape");
}

/// @brief bandpass filler  
/// @details
/// @param[in] bp pair of cubes with bandpasses and validity flags (to be resised to (2*nChan) x nAnt x nBeam)
void TableCalSolutionFiller::fillBandpasses(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &bp) const
{
  if (itsCreateNew || noBandpass()) {
      ASKAPDEBUGASSERT(itsBandpassesRow < 0);
      bp.first.resize(2 * itsNChan, itsNAnt, itsNBeam);
      bp.first.set(1.);
      bp.second.resize(2 * itsNChan, itsNAnt, itsNBeam);
      bp.second.set(false);
      itsBandpassesRow = itsRefRow;      
  } else {  
     if (itsBandpassesRow < 0) {
         itsBandpassesRow = findDefinedCube("BANDPASS");
     }
     ASKAPASSERT(itsBandpassesRow>=0);
     ASKAPCHECK(cellDefined<casa::Bool>("BANDPASS_VALID", casa::uInt(itsBandpassesRow)), 
         "Wrong format of the calibration table: BANDPASS element should always be accompanied by BANDPASS_VALID");
     readCube(bp.first, "BANDPASS", casa::uInt(itsBandpassesRow));
     readCube(bp.second, "BANDPASS_VALID", casa::uInt(itsBandpassesRow));
  }
  ASKAPCHECK(bp.first.shape() == bp.second.shape(), "BANDPASS and BANDPASS_VALID cubes are expected to have the same shape");
}
  
/// @brief gains writer
/// @details
/// @param[in] gains pair of cubes with gains and validity flags (should be 2 x nAnt x nBeam)
void TableCalSolutionFiller::writeGains(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &gains) const
{
  ASKAPASSERT(itsGainsRow>=0);
  ASKAPCHECK(gains.first.shape() == gains.second.shape(), "The cubes with gains and validity flags are expected to have the same shape");
  writeCube(gains.first, "GAIN", casa::uInt(itsGainsRow));
  writeCube(gains.second, "GAIN_VALID", casa::uInt(itsGainsRow));    
}
  
/// @brief leakage writer  
/// @details
/// @param[in] leakages pair of cubes with leakages and validity flags (should be 2 x nAnt x nBeam)
void TableCalSolutionFiller::writeLeakages(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &leakages) const
{
  ASKAPASSERT(itsLeakagesRow>=0);
  ASKAPCHECK(leakages.first.shape() == leakages.second.shape(), "The cubes with leakages and validity flags are expected to have the same shape");
  writeCube(leakages.first, "LEAKAGE", casa::uInt(itsLeakagesRow));
  writeCube(leakages.second, "LEAKAGE_VALID", casa::uInt(itsLeakagesRow));    
}

/// @brief bandpass writer  
/// @details
/// @param[in] bp pair of cubes with bandpasses and validity flags (should be (2*nChan) x nAnt x nBeam)
void TableCalSolutionFiller::writeBandpasses(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &bp) const
{
  ASKAPASSERT(itsBandpassesRow>=0);
  ASKAPCHECK(bp.first.shape() == bp.second.shape(), "The cubes with bandpasses and validity flags are expected to have the same shape");
  writeCube(bp.first, "BANDPASS", casa::uInt(itsBandpassesRow));
  writeCube(bp.second, "BANDPASS_VALID", casa::uInt(itsBandpassesRow));    
}

/// @brief find first defined cube searching backwards
/// @details This assumes that the table rows are given in the time order. If the cell at the reference row
/// doesn't have a cube defined, the search is continued up to the top of the table. An exception is thrown
/// if no defined cube has been found.
/// @param[in] name column name
/// @return row number for a defined cube
/// @note The code always returns non-negative number.
long TableCalSolutionFiller::findDefinedCube(const std::string &name) const
{
  for (long tempRow = itsRefRow; tempRow >= 0; --tempRow) {
       if (cellDefined<casa::Complex>(name, casa::uInt(tempRow))) {
           return tempRow;
       }
  }
  ASKAPTHROW(AskapError, "Unable to find valid element in column "<<name<<" at row "<<itsRefRow<<" or earlier");
}


} // namespace accessors

} // namespace askap


