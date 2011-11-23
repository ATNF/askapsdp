/// @file 
///
/// @brief MS filler and the result buffer manager
/// @details This class manages a double buffer for the resulting visibilities and
/// flags. When the BAT timestamp changes, it stores the previous buffer.
/// It is intended to be run in a parallel thread.
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

#ifndef ASKAP_SWCORRELATOR_CORR_FILLER
#define ASKAP_SWCORRELATOR_CORR_FILLER

// own includes
#include <swcorrelator/CorrProducts.h>

// other 3rd party
#include <Common/ParameterSet.h>

// std includes
#include <vector>
#include <utility>

// boost includes
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

namespace askap {

namespace swcorrelator {

/// @brief MS filler and the result buffer manager
/// @details This class manages a double buffer for the resulting visibilities and
/// flags. When the BAT timestamp changes, it stores the previous buffer.
/// It is intended to be run in a parallel thread.
/// @ingroup swcorrelator
class CorrFiller {
public:
  /// @brief constructor, sets up the filler
  /// @details Configuration is done via the parset
  /// @param[in] parset parset file with configuration info
  CorrFiller(const LOFAR::ParameterSet &parset);
  
  /// @brief maximum number of antennas
  /// @return number of antennas
  inline int nAnt() const { return itsNAnt; }
  
  /// @brief maximum number of beams
  /// @return maximum number of beams
  inline int nBeam() const {return itsNBeam;}
  
  /// @brief maximum number of spectral channels
  /// @return maximum number of spectral channels (or cards)
  inline int nChan() const {return itsNChan;}
  
  /// @brief shutdown the filler
  /// @detais This method is effectively a destructor, which can be
  /// called more explicitly. It stops the writing thread and
  /// closes the MS which is currently being written.
  void shutdown();
  
  /// @brief manage the flush status flag
  
  /// @brief get buffer to write it to MS
  /// @details This method is intended to be called from the writing thread. It obtains
  /// a buffer corresponding to the given beam. It is assumed the required locks have
  /// already been obtained.
  /// @param[in] beam beam of interest
  /// @oaram[in] useFirst true, if the first set of buffers is flushed, false otherwise
  /// @return reference to the buffer
  CorrProducts& getProductsToWrite(const int beam, const bool useFirst) const;

  /// @brief get job for writing 
  /// @details This method is intended to be called from the writing thread. It acquires
  /// the required locks (and block the thread until it happens). The method returns
  /// the flag showing which buffer set is to be written.
  /// @return true, if the first buffer is to be written, false otherwise
  bool getWritingJob();
  
  /// @brief notify that the writing job has been finished
  /// @details This method is intended to be called from the writing thread. It releases
  /// the whole set of buffers unlocking them for the corelation threads.
  /// @param[in] isFirst true, if the first set of buffers is released, false otherwise
  void notifyWritingDone(const bool useFirst);
    
  /// @brief get buffer to be fileld with new data
  /// @details This method is intended to be called from correlator threads when
  /// new visibility data are ready to be stored
  /// @param[in] beam beam index
  /// @param[in] bat time stamp
  /// @return reference to the buffer
  /// @note it calls notifyOfNewData
  CorrProducts& productsBuffer(const int beam, const uint64_t bat);
  
  /// @brief notify that the buffer has been filled with data
  /// @param[in] beam beam index
  void notifyProductsReady(const int beam);

protected:
  
  /// @brief notify that the new data are about to be received
  /// @details This method is intended to be called from correlator threads when
  /// a new piece of data is about to be stored in a buffer. It triggers write and
  /// a buffer swap if the new BAT is different from that of the buffer.
  /// @param[in] bat time stamp of the new data
  void notifyOfNewData(const uint64_t bat);
  
  /// @brief wait for all fill operations to complete
  /// @details This method waits for all buffer fill operations to complete,
  /// this is necessary before the buffer swap could be initiated and the
  /// current buffer could be transfered to the writing thread to store.
  void waitFillCompletion();
  
private:
 
  /// @brief maximum number of antennas (should always be 3 for now)
  int itsNAnt;
  
  /// @brief maximum number of beams
  int itsNBeam;
  
  /// @brief maximum number of spectral channels (or cards)
  int itsNChan;
  
  /// @brief two products for every beam (active and standby)
  std::vector<boost::shared_ptr<CorrProducts> > itsCorrProducts;
  
  /// @brief status of the first and the second buffers
  /// @details We have two buffers per beam. When BAT changes, the current 
  /// content is made ready to be flushed to disk. The flag is true
  /// if the appropriate set of buffers (i.e. first or second) is currently
  /// being written to disk by the parallel thread and false otherwise.
  /// The transition to flush status (i.e. true) requires to wait for all
  /// writing to be complete
  std::pair<bool, bool> itsFlushStatus;
  
  /// @brief true if the first set of buffers is active for writing
  bool itsFirstActive;
  
  /// @brief time corresponding to the active buffer (i.e what is correlated now)
  /// @note uint64_t(-1) is a flag of an uninitialised BAT (i.e. active buffer is fresh)
  uint64_t  itsActiveBAT;
  
  /// @brief true if flush is requested
  /// @details This flag is monitored (through the condition variable) by the writing thread. If it is
  /// raised, the active buffer is deactivated and write operation commences.
  bool itsReadyToWrite;
  
  /// @brief status of the current active buffers
  /// @details The writing is done in parallel, separately for each beam.
  /// Therefore, we keep status flag per beam. true means the appropriate
  /// buffer is being filled.
  std::vector<bool> itsFillStatus;

  /// @brief status condition variable
  mutable boost::condition_variable itsStatusCV;
  
  /// @brief mutex associated with the status condition variable
  mutable boost::mutex itsStatusCVMutex;

  /// @brief true if buffer swap is being handled
  /// @details We have multiple writing threads and have to unlock the mutex to wait
  /// for all threads to finish. To avoid any of these writing threads to jump into
  /// buffer swap mechanism, this condition is checked before swap is initiated.  
  bool itsSwapHandled;
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_CORR_FILLER



