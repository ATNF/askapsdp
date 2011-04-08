/// @file BandpassSolution.h
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

#ifndef ASKAP_CP_CPDATASERVICES_BANDPASSSOLUTION_H
#define ASKAP_CP_CPDATASERVICES_BANDPASSSOLUTION_H

// System includes

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "casa/Arrays/Cube.h"
#include "casa/Arrays/Vector.h"

// Local package includes
#include "calibrationclient/JonesJTerm.h"

namespace askap {
namespace cp {
namespace caldataservice {

class BandpassSolution {

    public:
        /// Constructor
        BandpassSolution(const casa::Long timestamp,
                         const casa::Short nAntenna,
                         const casa::Short nBeam,
                         const casa::Int nChan);

        const casa::Cube<JonesJTerm>& bandpass(void) const;
        casa::Cube<JonesJTerm>& bandpass(void);

        const casa::Vector<casa::Int>& antennaIndex(void) const;
        casa::Vector<casa::Int>& antennaIndex(void);

        const casa::Vector<casa::Int>& beamIndex(void) const;
        casa::Vector<casa::Int>& beamIndex(void);

        const casa::Vector<casa::Int>& chanIndex(void) const;
        casa::Vector<casa::Int>& chanIndex(void);

    private:
        casa::Long itsTimestamp;
        casa::Short itsNAntenna;
        casa::Short itsNBeam;
        casa::Short itsNChan;
        casa::Cube<JonesJTerm> itsBandpass;
        casa::Vector<casa::Int> itsAntennaIndex;
        casa::Vector<casa::Int> itsBeamIndex;
        casa::Vector<casa::Int> itsChanIndex;
};

};
};
};

#endif
