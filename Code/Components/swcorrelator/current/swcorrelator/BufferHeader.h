/// @file 
///
/// @brief header of the buffer of data send via TCP connection
/// @details This header preceds any actual data in the stream.
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

#ifndef ASKAP_SWCORRELATOR_BUFFER_HEADER
#define ASKAP_SWCORRELATOR_BUFFER_HEADER

#include <inttypes.h>

namespace askap {

namespace swcorrelator {

/// @brief header preceding each data chunk for all buffers
/// @details This header preceds any actual data in the stream.
/// @ingroup swcorrelator
struct BufferHeader {
    uint64_t    bat;     // BAT of first sample
    uint32_t    antenna; // antenna number
    uint32_t    freqId;  // currently card number 1-16.  If we go to more DSPs will be 1-64 (same as FreqId field is FITs acm capture)
    uint32_t    beam;    // beam number 1-18
    uint32_t    frame;   // frame number of first sample (from digitiser packet, see below)
    uint32_t    control; // user defined value, can be hooked up thru epics to send control via OSL script or gui to software correlator if necessary
    uint32_t    length;  // length of data (should always be 4194304 (num ddr pages * bytes per beam) for 16MHz
};


} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_BUFFER_HEADER


