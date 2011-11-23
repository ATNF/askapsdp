/// @file 
///
/// @brief Writing thread of the MS filler
/// @details This class holds a shared pointer to the main filler and can call
/// its methods to get data and to synchronise.
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

#include <swcorrelator/FillerWorker.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>

ASKAP_LOGGER(logger, ".corrfiller");

#include <string>

namespace askap {

namespace swcorrelator {

/// @brief constructor, pass the shared pointer to the filler
FillerWorker::FillerWorker(const boost::shared_ptr<CorrFiller> &filler) : itsFiller(filler) 
{
  ASKAPDEBUGASSERT(itsFiller);
}

/// @brief entry point for the parallel thread
void FillerWorker::operator()()
{
  ASKAPLOG_INFO_STR(logger, "Writing thread started");
  while (true) {
     const bool buffer = itsFiller->getWritingJob();
     const std::string bufType = buffer ? "first" : "second";
     for (int beam=0; beam < itsFiller->nBeam(); ++beam) {
          CorrProducts &cp = itsFiller->getProductsToWrite(beam, buffer);
          ASKAPLOG_INFO_STR(logger, "Write for buffer `"<<bufType<<"` beam="<<beam<<" bat="<<cp.itsBAT<<
             "vis="<<cp.itsVisibility<<" flag="<<cp.itsFlag);
     }
     itsFiller->notifyWritingDone(buffer);
  }
}

} // namespace swcorrelator

} // namespace askap