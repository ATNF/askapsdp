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

#include <swcorrelator/CorrWorker.h>
#include <swcorrelator/SimpleCorrelator.h>
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <boost/thread.hpp>

ASKAP_LOGGER(logger, ".corrworker");

namespace askap {

namespace swcorrelator {

/// @brief constructor
/// @details 
/// @param[in] filler shared pointer to a filler
/// @param[in] bm shared pointer to a buffer manager
CorrWorker::CorrWorker(const boost::shared_ptr<CorrFiller> &filler,
           const boost::shared_ptr<BufferManager> &bm) : itsFiller(filler), itsBufferManager(bm)
{
  ASKAPDEBUGASSERT(itsFiller);
  ASKAPDEBUGASSERT(itsBufferManager);
}             

/// @brief entry point for the parallel thread
void CorrWorker::operator()() 
{
  ASKAPLOG_INFO_STR(logger, "Correlator thread started, id="<<boost::this_thread::get_id());
  try {
    ASKAPDEBUGASSERT(itsFiller);
    ASKAPDEBUGASSERT(itsBufferManager);
    while (true) {
       // just for debugging generate a fake result every second (asynchronously - so can't debug multiple beams/channels)
       boost::this_thread::sleep(boost::posix_time::seconds(1));
       const uint64_t bat = uint64_t(time(0));
       CorrProducts& cp = itsFiller->productsBuffer(0, bat);
       cp.itsBAT = bat;
       itsFiller->notifyProductsReady(0);
    }
  } catch (const AskapError &ae) {
     ASKAPLOG_FATAL_STR(logger, "Correlator thread (id="<<boost::this_thread::get_id()<<") is about to die: "<<ae.what());
     throw;
  }
  
}

} // namespace swcorrelator

} // namespace askap

