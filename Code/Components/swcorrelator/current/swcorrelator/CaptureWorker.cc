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

#include <swcorrelator/CaptureWorker.h>
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <boost/thread.hpp>
#include <askap/AskapUtil.h>

ASKAP_LOGGER(logger, ".captureworker");

namespace askap {

namespace swcorrelator {

/// @brief constructor
/// @details 
/// @param[in] bm shared pointer to a buffer manager
CaptureWorker::CaptureWorker(const boost::shared_ptr<BufferManager> &bm) :
    itsBufferManager(bm) {}

/// @brief entry point for the parallel thread
void CaptureWorker::operator()()
{
   ASKAPLOG_INFO_STR(logger, "Capture worker thread started, id="<<boost::this_thread::get_id());
   try {
     ASKAPDEBUGASSERT(itsBufferManager);
     // buffer size in complex floats
     const int size = (itsBufferManager->bufferSize() - int(sizeof(BufferHeader))) / sizeof(float) / 2;
     while (true) {
       // extract the first complete buffer
       int id = itsBufferManager->getFilledBuffer();
       const uint64_t bat = itsBufferManager->header(id).bat;
       const int beam = itsBufferManager->header(id).beam;
       const int chan = itsBufferManager->header(id).freqId;
       const int antenna = itsBufferManager->header(id).antenna;
       const std::string fname = "ant"+utility::toString<int>(antenna)+".beam"+utility::toString<int>(beam)+
            ".chan"+utility::toString<int>(chan)+".bat"+utility::toString<uint64_t>(bat)+".dat";
       ASKAPLOG_INFO_STR(logger, "About to dump the data to the file "<<fname);
       // format is just number of complex words followed by real, imaginary, ...
       std::ofstream os(fname.c_str());
       os.write((char*)&size, sizeof(int));
       std::complex<float>* data = itsBufferManager->data(id);
       for (int i=0; i<size; ++i,++data) {
            float buf = real(*data);
            os.write((char*)&buf, sizeof(float));
            buf = imag(*data);
            os.write((char*)&buf, sizeof(float));
       }              
       itsBufferManager->releaseBuffers(id);
     }
  } catch (const AskapError &ae) {
     ASKAPLOG_FATAL_STR(logger, "Correlator thread (id="<<boost::this_thread::get_id()<<") is about to die: "<<ae.what());
     throw;
  }
    
}

} // namespace swcorrelator

} // namespace askap