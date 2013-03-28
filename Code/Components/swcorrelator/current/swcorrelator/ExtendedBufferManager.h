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

#ifndef ASKAP_SWCORRELATOR_EXTENDED_BUFFER_MANAGER
#define ASKAP_SWCORRELATOR_EXTENDED_BUFFER_MANAGER

// own includes
#include <swcorrelator/BufferManager.h>
#include <swcorrelator/HeaderPreprocessor.h>

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// std includes
#include <vector>
#include <utility>


namespace askap {

namespace swcorrelator {

/// @brief manages buffers for raw data
/// @details This class provides support for more than 3 baselines. Although
/// the original BufferManager accepts the number of antennas as its parameter,
/// it is only used to resize the storage accordingly. The logic to split 
/// the set of baselines into 3-antenna triangles is implemented here.
/// @ingroup swcorrelator
class ExtendedBufferManager : public BufferManager {
public:

   /// @brief constructor
   /// @details
   /// @param[in] nBeam number of beams
   /// @param[in] nChan number of channels (cards)
   /// @param[in] nAnt number of antennas
   /// @param[in[ hdrProc optional shared pointer to the header preprocessor
   ExtendedBufferManager(const size_t nBeam, const size_t nChan, const size_t nAnt = 3, 
         const boost::shared_ptr<HeaderPreprocessor> &hdrProc = boost::shared_ptr<HeaderPreprocessor>());
   
   /// @brief get filled buffers for a matching channel + beam
   /// @details This method returns the first available set of
   /// completely filled buffers corresponding to the same channel
   /// and beam. The calling thread is blocked until a suitable set
   /// is available for correlation.
   /// @return a set of buffers ready for correlation
   virtual BufferSet getFilledBuffers() const;
   
   /// @brief release the buffers
   /// @details This method notifies the manager that correlation is
   /// now complete and the data buffers can now be released.
   /// @param[in] ids buffer set to release
   /// @note this version of the method is called from correlator thread
   /// to get the next work unit. It is made virtual to be able to change
   /// the behavior for more than 3 antenna case. Other overloaded 
   /// versions do not need this polymorphism and are therefore non-virtual
   virtual void releaseBuffers(const BufferSet &ids) const;

protected:
   /// @brief process a new complete set of antennas
   /// @details This method called every time a new complete set of per-antenna 
   /// buffers is found. It is intended to be overridden in derived classes to support
   /// more than 3 antennas (by building the appropriate strategy of iterating over the
   /// baseline space).
   /// @param[in] index channel/beam pair
   /// @return buffer set structure with buffer indices corresponding to the first chosen triangle
   /// @note it is implied that the required locks have already been obtained
   virtual BufferSet newBufferSet(const std::pair<int,int> &index) const;
   
   /// @brief helper method to check that some baseline triangles are still processed
   /// @return true if at least one triangle is still being processed
   /// @note It is assumed that the lock had been aquired
   bool notAllReleased() const;

   /// @brief helper method to get a triangle according to the iteration plan
   /// @param[in] index item in the plan
   /// @return buffer set filled with buffer IDs
   BufferSet getTriangle(const int index) const; 
      
private:
      
   /// @brief mutex protecting the data of this class
   mutable boost::mutex itsGroupMutex;
     
   /// @brief condition variable for release counters
   mutable boost::condition_variable itsReleaseCV;
   
   /// @brief current group
   /// @details This is essentially a counter of BufferSets returned by this class. 
   /// when it reaches the number of groups, a new complete set of antennas is requested from the 
   /// parent class (which may belong to a different channel/beam). Negative number means
   /// the class has just been initialised and didn't return any group yet
   mutable int itsGroupCounter;
   
   /// @brief buffers for each antenna for the channel/beam being processed
   /// @details Negative value means the numbers have not been initialised.
   mutable casa::Vector<int> itsBuffers;
   
   /// @brief iteration plan
   /// @details Each BufferSet stores antenna indices (after pre-processing) rather than buffer indices.
   /// The number of elements is the number of groups. The content is initialised in the contructor
   /// (using the number of antennas) and then remains unchanged. The returned set is formed using
   /// itsBuffers and itsGroupCounter element of this plan.
   std::vector<BufferSet> itsPlan;
   
   /// @brief release counters
   /// @details There is one item per baseline set stored in itsPlan. The element is true if a particular
   /// combination is not released. All elements false means that this channel/beam is either done or not started
   mutable casa::Vector<bool> itsReleaseFlags;
      
}; // class ExtendedBufferManager


} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_EXTENDED_BUFFER_MANAGER


