/// @file TosMetadata.h
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

#ifndef ASKAP_CP_TOSMETADATA_H
#define ASKAP_CP_TOSMETADATA_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "casa/aips.h"

// Local package includes
#include "cpcommon/TosMetadataAntenna.h"

namespace askap {
    namespace cp {
        class TosMetadata
        {
            public:
                TosMetadata(const casa::uInt& nCoarseChannels,
                        const casa::uInt& nBeams,
                        const casa::uInt& nPol);

                /////////////////////
                // Getters
                /////////////////////

                casa::uInt nAntenna(void) const;

                casa::uInt nCoarseChannels(void) const;

                casa::uInt nBeams(void) const;

                casa::uInt nPol(void) const;

                casa::uLong time(void) const;

                casa::uLong period(void) const;

                /////////////////////
                // Setters
                /////////////////////

                void time(const casa::uLong& time);

                void period(const casa::uLong& period);


                /////////////////////////
                // Antenna access methods
                /////////////////////////

                casa::uInt addAntenna(const casa::String& name);

                const TosMetadataAntenna& antenna(const casa::uInt id) const;

                TosMetadataAntenna& antenna(const casa::uInt id);

            private:
                void checkAntennaId(const casa::uInt& id) const;

                casa::uInt itsNumCoarseChannels;
                casa::uInt itsNumBeams;
                casa::uInt itsNumPol;

                casa::uLong itsTime;
                casa::uLong itsPeriod;

                std::vector<TosMetadataAntenna> itsAntenna;
        };

    }
}

#endif
