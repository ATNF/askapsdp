/// @file VisElement.h
///
/// @copyright (c) 2014 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_VISPUBLISHER_VISELEMENT_T
#define ASKAP_CP_VISPUBLISHER_VISELEMENT_T

// System includes
#include <stdint.h>

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief Contains the Vis summary data (amp, phase, delay) for a given
/// baseline, beam and polarisation.
struct VisElement {
    // Beam ID (zero based)
    uint32_t beam;

    // Antenna 1 index (zero based)
    uint32_t antenna1;

    // Antenna 2 index (zero based)
    uint32_t antenna2;

    // Polarisation
    // 0=XX, 1=XY, 2=YX, 3=YY
    uint32_t pol;

    // Average amplitude
    // Unit: Pseudo Jy
    double amplitude;

    // Average Phase Angle
    // Unit: Degrees
    double phase;

    // Delays
    // Unit: Seconds
    double delay;
};

}
}
}

#endif
