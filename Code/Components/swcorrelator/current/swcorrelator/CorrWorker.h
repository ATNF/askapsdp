/// @file 
///
/// @brief Thread which does correlation
/// @details This class holds shared pointers to the filler and the buffer
/// manager. The parallel thread extracts data corresponding to all three 
/// baselines, some spectral channel and beam, correlates them and passes
/// to the filler for writing. The filler and buffer manager manage 
/// synchronisation.
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

#ifndef ASKAP_SWCORRELATOR_CORR_WORKER
#define ASKAP_SWCORRELATOR_CORR_WORKER

#include <boost/shared_ptr.hpp>
#include <swcorrelator/BufferManager.h>
#include <swcorrelator/CorrFiller.h>

namespace askap {

namespace swcorrelator {

/// @brief Thread which does correlation
/// @details This class holds shared pointers to the filler and the buffer
/// manager. The parallel thread extracts data corresponding to all three 
/// baselines, some spectral channel and beam, correlates them and passes
/// to the filler for writing. The filler and buffer manager manage 
/// synchronisation.
/// @ingroup swcorrelator
struct CorrWorker {

  /// @brief constructor
  /// @details 
  /// @param[in] filler shared pointer to a filler
  /// @param[in] bm shared pointer to a buffer manager
  CorrWorker(const boost::shared_ptr<CorrFiller> &filler,
             const boost::shared_ptr<BufferManager> &bm);

  /// @brief entry point for the parallel thread
  void operator()();
  
private:
  /// @brief filler
  boost::shared_ptr<CorrFiller> itsFiller;
  /// @brief buffer manager
  boost::shared_ptr<BufferManager> itsBufferManager;  
};


} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_CORR_WORKER

