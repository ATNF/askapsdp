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

// System includes
#include <string>

// ASKAPsoft includes
#include "casa/aips.h"

// Local package includes

namespace askap {
namespace cp {
namespace skymodelservice {

// Component identifier typedef
typedef long ComponentId;

class Component {

    public:

        Component(const ComponentId id,
                  double rightAscension,
                  double declination,
                  double positionAngle,
                  double majorAxis,
                  double minorAxis,
                  double i1400);

        /**
         * Unique component index number
         */
        ComponentId id() const;

        /**
         * Right ascension in the J2000 coordinate system
         * Units: degrees
         */
        double rightAscension() const;

        /**
         * Declination in the J2000 coordinate system
         * Units: degrees
         */
        double declination() const;

        /**
         * Position angle. Counted east from north.
         * Units: radians
         */
        double positionAngle() const;

        /**
         * Major axis
         * Units: arcsecs
         */
        double majorAxis() const;

        /**
         * Minor axis
         * Units: arcsecs
         */
        double minorAxis() const;

        /**
         * Flux at 1400 Mhz
         * Units: Jy (log10 of flux in Jy???)
         */
        double i1400() const;


    private:
        ComponentId itsId;
        double itsRightAscension;
        double itsDeclination;
        double itsPositionAngle;
        double itsMajorAxis;
        double itsMinorAxis;
        double itsI1400;
};

};
};
};

#endif
