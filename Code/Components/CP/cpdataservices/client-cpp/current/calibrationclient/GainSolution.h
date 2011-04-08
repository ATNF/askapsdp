/// @file GainSolution.h
///
/// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_CP_CPDATASERVICES_GAINSOLUTION_H
#define ASKAP_CP_CPDATASERVICES_GAINSOLUTION_H

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"

// Local package includes
#include "calibrationclient/JonesJTerm.h"

namespace askap {
namespace cp {
namespace caldataservice {

class GainSolution {

    public:
        /// Constructor
        GainSolution(const casa::Long timestamp,
                     const casa::Short nAntenna,
                     const casa::Short nBeam);

        const casa::Matrix<JonesJTerm>& gains(void) const;
        casa::Matrix<JonesJTerm>& gains(void);

        const casa::Vector<casa::Int>& antennaIndex(void) const;
        casa::Vector<casa::Int>& antennaIndex(void);

        const casa::Vector<casa::Int>& beamIndex(void) const;
        casa::Vector<casa::Int>& beamIndex(void);

    private:
        casa::Long itsTimestamp;
        casa::Short itsNAntenna;
        casa::Short itsNBeam;
        casa::Matrix<JonesJTerm> itsGains;
        casa::Vector<casa::Int> itsAntennaIndex;
        casa::Vector<casa::Int> itsBeamIndex;
};

};
};
};

#endif
