/// @file Component.h
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

#ifndef ASKAP_CP_SKYMODELSERVICE_COMPONENT_H
#define ASKAP_CP_SKYMODELSERVICE_COMPONENT_H

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "casa/Quanta/Quantum.h"

namespace askap {
namespace cp {
namespace skymodelservice {

// Component identifier typedef
typedef casa::Long ComponentId;

class Component {

    public:

        Component(const ComponentId id,
                  casa::Quantity rightAscension,
                  casa::Quantity declination,
                  casa::Quantity positionAngle,
                  casa::Quantity majorAxis,
                  casa::Quantity minorAxis,
                  casa::Quantity i1400);

        /**
         * Unique component index number
         */
        ComponentId id() const;

        /**
         * Right ascension in the J2000 coordinate system
         * Base units: degrees
         */
        casa::Quantity rightAscension() const;

        /**
         * Declination in the J2000 coordinate system
         * Base units: degrees
         */
        casa::Quantity declination() const;

        /**
         * Position angle. Counted east from north.
         * Base units: radians
         */
        casa::Quantity positionAngle() const;

        /**
         * Major axis
         * Base units: arcsecs
         */
        casa::Quantity majorAxis() const;

        /**
         * Minor axis
         * Base units: arcsecs
         */
        casa::Quantity minorAxis() const;

        /**
         * Flux at 1400 Mhz
         * Base units: Jy
         */
        casa::Quantity i1400() const;


    private:
        ComponentId itsId;
        casa::Quantity itsRightAscension;
        casa::Quantity itsDeclination;
        casa::Quantity itsPositionAngle;
        casa::Quantity itsMajorAxis;
        casa::Quantity itsMinorAxis;
        casa::Quantity itsI1400;
};

};
};
};

#endif
