/// @file 
///
/// @brief Thread which just dumps the data into binary file
/// @details This class holds shared pointer to the buffer
/// manager. The parallel thread extracts data when a new buffer is ready
/// and then dumps the content into a file. This is an anternative to the
/// correlation thread and they shouldn't be launched together (or there 
/// will be a data race).
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

#ifndef ASKAP_SWCORRELATOR_CAPTURE_WORKER
#define ASKAP_SWCORRELATOR_CAPTURE_WORKER

#include <boost/shared_ptr.hpp>
#include <swcorrelator/BufferManager.h>

namespace askap {

namespace swcorrelator {

/// @brief Thread which just dumps the data into binary file
/// @details This class holds shared pointer to the buffer
/// manager. The parallel thread extracts data when a new buffer is ready
/// and then dumps the content into a file. This is an anternative to the
/// correlation thread and they shouldn't be launched together (or there 
/// will be a data race).
/// @ingroup swcorrelator
struct CaptureWorker {

  /// @brief constructor
  /// @details 
  /// @param[in] bm shared pointer to a buffer manager
  CaptureWorker(const boost::shared_ptr<BufferManager> &bm);

  /// @brief entry point for the parallel thread
  void operator()();
  
private:
  /// @brief buffer manager
  boost::shared_ptr<BufferManager> itsBufferManager;  
};


} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_CAPTURE_WORKER

