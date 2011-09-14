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

/// @brief construct the object and link it to the given table
/// @param[in] tab  table to use
TableCalSolutionFiller::TableCalSolutionFiller(const casa::Table& tab) : TableHolder(tab), TableBufferManager(tab) {}

/// @brief gains filler  
/// @details
/// @param[in] gains pair of cubes with gains and validity flags (to be resised to 2 x nAnt x nBeam)
void TableCalSolutionFiller::fillGains(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &gains) const
{
}
  
/// @brief leakage filler  
/// @details
/// @param[in] leakages pair of cubes with leakages and validity flags (to be resised to 2 x nAnt x nBeam)
void TableCalSolutionFiller::fillLeakages(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &leakages) const
{
}

/// @brief bandpass filler  
/// @details
/// @param[in] bp pair of cubes with bandpasses and validity flags (to be resised to (2*nChan) x nAnt x nBeam)
void TableCalSolutionFiller::fillBandpasses(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &bp) const
{
}
  
/// @brief gains writer
/// @details
/// @param[in] gains pair of cubes with gains and validity flags (should be 2 x nAnt x nBeam)
void TableCalSolutionFiller::writeGains(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &gains) const
{
}
  
/// @brief leakage writer  
/// @details
/// @param[in] leakages pair of cubes with leakages and validity flags (should be 2 x nAnt x nBeam)
void TableCalSolutionFiller::writeLeakages(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &leakages) const
{
}

/// @brief bandpass writer  
/// @details
/// @param[in] bp pair of cubes with bandpasses and validity flags (should be (2*nChan) x nAnt x nBeam)
void TableCalSolutionFiller::writeBandpasses(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &bp) const
{
}


} // namespace accessors

} // namespace askap


