/// @file 
///
/// @brief manages buffers for raw data
/// @details This class provides support for more than 3 baselines. Although
/// the original BufferManager accepts the number of antennas as its parameter,
/// it is only used to resize the storage accordingly. The logic to split 
/// the set of baselines into 3-antenna triangles is implemented here.
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

#include <swcorrelator/ExtendedBufferManager.h>
#include <askap/AskapError.h>
#include <swcorrelator/CorrProducts.h>
#include <boost/thread/thread.hpp>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>


ASKAP_LOGGER(logger, ".swcorrelator");

namespace askap {

namespace swcorrelator {

/// @brief constructor
/// @details
/// @param[in] nBeam number of beams
/// @param[in] nChan number of channels (cards)
/// @param[in] nAnt number of antennas
/// @param[in[ hdrProc optional shared pointer to the header preprocessor
ExtendedBufferManager::ExtendedBufferManager(const size_t nBeam, const size_t nChan, const size_t nAnt, 
         const boost::shared_ptr<HeaderPreprocessor> &hdrProc) : BufferManager(nBeam, nChan, nAnt, hdrProc),
         itsGroupCounter(-1), itsBuffers(nAnt, -1) 
{
  ASKAPDEBUGASSERT(nAnt >= 3);
  // worst case scenario, we have less groups than the baselines
  itsPlan.reserve(nAnt * (nAnt - 1) / 2); 
}
   
/// @brief get filled buffers for a matching channel + beam
/// @details This method returns the first available set of
/// completely filled buffers corresponding to the same channel
/// and beam. The calling thread is blocked until a suitable set
/// is available for correlation.
/// @return a set of buffers ready for correlation
BufferManager::BufferSet ExtendedBufferManager::getFilledBuffers() const
{
  return BufferManager::getFilledBuffers();
}
   
/// @brief release the buffers
/// @details This method notifies the manager that correlation is
/// now complete and the data buffers can now be released.
/// @param[in] ids buffer set to release
/// @note this version of the method is called from correlator thread
/// to get the next work unit. It is made virtual to be able to change
/// the behavior for more than 3 antenna case. Other overloaded 
/// versions do not need this polymorphism and are therefore non-virtual
void ExtendedBufferManager::releaseBuffers(const BufferSet &ids) const
{
  BufferManager::releaseBuffers(ids);
}

/// @brief process a new complete set of antennas
/// @details This method called every time a new complete set of per-antenna 
/// buffers is found. It is intended to be overridden in derived classes to support
/// more than 3 antennas (by building the appropriate strategy of iterating over the
/// baseline space).
/// @param[in] index channel/beam pair
/// @return buffer set structure with buffer indices corresponding to the first chosen triangle
/// @note it is implied that the required locks have already been obtained
BufferManager::BufferSet ExtendedBufferManager::newBufferSet(const std::pair<int,int> &index) const
{
  return BufferManager::newBufferSet(index);
}


} // namespace swcorrelator

} // namespace askap

