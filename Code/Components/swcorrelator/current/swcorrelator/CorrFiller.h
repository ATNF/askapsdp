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
  
  /// @brief status of the current active buffers
  /// @details The writing is done in parallel, separately for each beam.
  /// Therefore, we keep status flag per beam. true means the appropriate
  /// buffer is being filled.
  std::vector<bool> itsFillStatus;
  
  /// @brief status condition variable
  mutable boost::condition_variable itsStatusCV;
  
  /// @brief mutex associated with the status condition variable
  mutable boost::mutex itsStatusCVMutex;
  
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_CORR_FILLER



