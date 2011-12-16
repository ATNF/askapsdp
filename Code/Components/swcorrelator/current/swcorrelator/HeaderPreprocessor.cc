/// @file 
///
/// @brief low-level operations on the header
/// @details We need some configurable flexibility dealing with the incoming stream. 
/// This class encapsulates low-level hacking operations on the buffer header to
/// allow necessary substutions. Index manipulation is done via IndexConverter.
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

#include <swcorrelator/HeaderPreprocessor.h>
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>

ASKAP_LOGGER(logger, ".swcorrelator");

namespace askap {

namespace swcorrelator {

/// @brief constructor, extracts parameters from the parset
/// @details
/// @param[in] parset parset with parameters
HeaderPreprocessor::HeaderPreprocessor(const LOFAR::ParameterSet &parset) : 
   itsSwapBeamAndAnt(parset.getBool("makeBeamAnt")) 
{
  ASKAPLOG_INFO_STR(logger,  "Setting up header preprocessing");
  if (itsSwapBeamAndAnt) {
      ASKAPLOG_INFO_STR(logger,  "Antennas will appear as beams and beams will appear as antennas");
  } 
  if (parset.isDefined("antmap")) {
      const std::string antMap = parset.getString("antmap");
      if (antMap != "") {
          ASKAPLOG_INFO_STR(logger,  "Antenna IDs will be mapped according to <"<<antMap<<">");
          itsAntIDConverter.reset(new IndexConverter(antMap));
      }
  }
  if (parset.isDefined("beammap")) {
      const std::string beamMap = parset.getString("beammap");
      if (beamMap != "") {
          ASKAPLOG_INFO_STR(logger,  "Beam IDs will be mapped according to <"<<beamMap<<">");
          itsBeamIDConverter.reset(new IndexConverter(beamMap));
      }
  }
  if (parset.isDefined("freqmap")) {
      const std::string freqMap = parset.getString("freqmap");
      if (freqMap != "") {
          ASKAPLOG_INFO_STR(logger,  "Frequency IDs will be mapped according to <"<<freqMap<<">");
          itsFreqIDConverter.reset(new IndexConverter(freqMap));
      }
  }
}   
  
/// @brief update the header
/// @param[in] hdr a reference to header
/// @return true if there is no valid mapping for this header and the data have to be rejected,
/// false otherwise
/// @note If the header is rejected, it remains unchanged (to help with the debugging). 
bool HeaderPreprocessor::updateHeader(BufferHeader &hdr) const
{
  int newAnt = itsSwapBeamAndAnt ? hdr.beam : hdr.antenna;
  int newBeam = itsSwapBeamAndAnt ? hdr.antenna : hdr.beam;
  int newFreqId = hdr.freqId;
  if (itsAntIDConverter) {
      newAnt = itsAntIDConverter->operator()(newAnt);
  }
  if (itsBeamIDConverter) {
      newBeam = itsBeamIDConverter->operator()(newBeam);
  }
  if (itsFreqIDConverter) {
      newFreqId = itsFreqIDConverter->operator()(newFreqId);
  }
  if ((newAnt < 0) || (newBeam < 0) || (newFreqId < 0)) {
      return true;
  }
  hdr.beam = newBeam;
  hdr.antenna = newAnt;
  hdr.freqId = newFreqId;
  return false;
}

} // namespace swcorrelator

} // namespace askap