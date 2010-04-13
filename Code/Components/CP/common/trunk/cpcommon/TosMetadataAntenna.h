/// @file TosMetadataAntenna.h
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_TOSMETADATAANTENNA_H
#define ASKAP_CP_TOSMETADATAANTENNA_H

// ASKAPsoft includes
#include "casa/aips.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Cube.h"
#include "measures/Measures/MDirection.h"

namespace askap {
namespace cp {

class TosMetadataAntenna {

    public:
        TosMetadataAntenna(const casa::String& name,
                           const casa::uInt& nCoarseChannels,
                           const casa::uInt& nBeams,
                           const casa::uInt& nPol);

        casa::String name(void) const;

        casa::uInt nCoarseChannels(void) const;

        casa::uInt nBeams(void) const;

        casa::uInt nPol(void) const;

        casa::MDirection dishPointing(void) const;

        void dishPointing(const casa::MDirection& val);

        casa::Double frequency(void) const;

        void frequency(const casa::Double& val);

        casa::String clientId(void) const;

        void clientId(const casa::String& val);

        casa::String scanId(void) const;

        void scanId(const casa::String& val);

        casa::MDirection phaseTrackingCentre(const casa::uInt& beam,
                             const casa::uInt& coarseChannel) const;

        void phaseTrackingCentre(const casa::MDirection& val,
                                 const casa::uInt& beam,
                                 const casa::uInt& coarseChannel);

        casa::Double parallacticAngle(void) const;

        void parallacticAngle(const casa::Double& val);

        casa::Bool onSource(void) const;

        void onSource(const casa::Bool& val);

        casa::Bool hwError(void) const;

        void hwError(const casa::Bool& val);

        casa::Bool flagDetailed(const casa::uInt& beam,
                                const casa::uInt& coarseChannel,
                                const casa::uInt& pol) const;

        void flagDetailed(const casa::Bool& val,
                          const casa::uInt& beam,
                          const casa::uInt& coarseChannel,
                          const casa::uInt& pol);

        casa::Float systemTemp(const casa::uInt& beam,
                               const casa::uInt& coarseChannel,
                               const casa::uInt& pol) const;

        void systemTemp(const casa::Float& val,
                        const casa::uInt& beam,
                        const casa::uInt& coarseChannel,
                        const casa::uInt& pol);

    private:

        void checkBeam(const casa::uInt& beam) const;

        void checkCoarseChannel(const casa::uInt& coarseChannel) const;

        void checkPol(const casa::uInt& pol) const;

        casa::String itsName;
        casa::uInt itsNumCoarseChannels;
        casa::uInt itsNumBeams;
        casa::uInt itsNumPol;

        casa::MDirection itsDishPointing;
        casa::Double itsFrequency;
        casa::String itsClientId;
        casa::String itsScanId;
        casa::Matrix<casa::MDirection> itsPhaseTrackingCentre;
        casa::Double itsParallacticAngle;
        casa::Bool itsOnSource;
        casa::Bool itsHwError;
        casa::Cube<casa::Bool> itsFlagDetailed;
        casa::Cube<casa::Float> itsSystemTemp;
};

}
}

#endif
