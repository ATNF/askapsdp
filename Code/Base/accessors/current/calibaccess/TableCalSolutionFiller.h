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

#ifndef TABLE_CAL_SOLUTION_FILLER_H
#define TABLE_CAL_SOLUTION_FILLER_H

// CASA includes
#include <tables/Tables/Table.h>

// own includes
#include <calibaccess/ICalSolutionFiller.h>
#include <dataaccess/TableBufferManager.h>

// std includes
#include <string>

namespace askap {

namespace accessors {

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
/// @ingroup calibaccess
class TableCalSolutionFiller : virtual protected TableBufferManager,
                               virtual public ICalSolutionFiller {
public:
  /// @brief construct the object and link it to the given table
  /// @details read-only operation is assumed
  /// @param[in] tab  table to use
  /// @param[in] row reference row
  TableCalSolutionFiller(const casa::Table& tab, const long row);

  /// @brief construct the object and link it to the given table
  /// @details Maximum allowed numbers of antennas, beams and spectral channels are
  /// set by this constructor which is essential for read-write operations (i.e. new
  /// table entries may need to be created
  /// @param[in] tab  table to use
  /// @param[in] row reference row
  /// @param[in] nAnt maximum number of antennas
  /// @param[in] nBeam maximum number of beams   
  /// @param[in] nChan maximum number of channels   
  TableCalSolutionFiller(const casa::Table& tab, const long row, const casa::uInt nAnt, 
         const casa::uInt nBeam, const casa::uInt nChan);

  /// @brief gains filler  
  /// @details
  /// @param[in] gains pair of cubes with gains and validity flags (to be resised to 2 x nAnt x nBeam)
  virtual void fillGains(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &gains) const;
  
  /// @brief leakage filler  
  /// @details
  /// @param[in] leakages pair of cubes with leakages and validity flags (to be resised to 2 x nAnt x nBeam)
  virtual void fillLeakages(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &leakages) const;

  /// @brief bandpass filler  
  /// @details
  /// @param[in] bp pair of cubes with bandpasses and validity flags (to be resised to (2*nChan) x nAnt x nBeam)
  virtual void fillBandpasses(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &bp) const;  
  
  /// @brief gains writer
  /// @details
  /// @param[in] gains pair of cubes with gains and validity flags (should be 2 x nAnt x nBeam)
  virtual void writeGains(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &gains) const;
  
  /// @brief leakage writer  
  /// @details
  /// @param[in] leakages pair of cubes with leakages and validity flags (should be 2 x nAnt x nBeam)
  virtual void writeLeakages(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &leakages) const;

  /// @brief bandpass writer  
  /// @details
  /// @param[in] bp pair of cubes with bandpasses and validity flags (should be (2*nChan) x nAnt x nBeam)
  virtual void writeBandpasses(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &bp) const;    

protected:

  /// @brief find first defined cube searching backwards
  /// @details This assumes that the table rows are given in the time order. If the cell at the reference row
  /// doesn't have a cube defined, the search is continued up to the top of the table. An exception is thrown
  /// if no defined cube has been found.
  /// @param[in] name column name
  /// @return row number for a defined cube
  /// @note The code always returns non-negative number.
  long findDefinedCube(const std::string &name) const;
  
  /// @brief helper method to check whether we are creating a new row  
  void checkForNewRow();
  
private:
  /// @brief number of antennas (used when new solutions are created)
  casa::uInt itsNAnt;
  /// @brief number of beams (used when new solutions are created)
  casa::uInt itsNBeam;
  /// @brief number of spectral channels (used when new solutions are created)
  casa::uInt itsNChan;
  /// @brief reference row for the selected solution (actual solution will be searched from this row up)
  long itsRefRow;

  // actual row numbers for gains, leakages and bandpasses (negative value means it is not yet determined)

  /// @brief row for gains
  mutable long itsGainsRow;

  /// @brief row for leakages
  mutable long itsLeakagesRow;
  
  /// @brief row for bandpasses
  mutable long itsBandpassesRow;   
  
  /// @brief true if a new row is to be created
  bool itsCreateNew; 
}; // class TableCalSolutionFiller

} // accessors

} // askap

#endif // #ifndef TABLE_CAL_SOLUTION_FILLER_H

