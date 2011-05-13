// @file SkyModelService.ice
//
// @copyright (c) 2011 CSIRO
// Australia Telescope National Facility (ATNF)
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// PO Box 76, Epping NSW 1710, Australia
// atnf-enquiries@csiro.au
//
// This file is part of the ASKAP software distribution.
//
// The ASKAP software distribution is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

#ifndef ASKAP_SKY_MODEL_SERVICE_ICE
#define ASKAP_SKY_MODEL_SERVICE_ICE

#include <CommonTypes.ice>

module askap
{
module interfaces
{
module skymodelservice
{
    /**
     * A component.
     **/
    struct Component
    {
        /**
         * Unique component index number
         **/
        long id;

        /**
         * Right ascension in the J2000 coordinate system
         * Units: degrees
         **/
        double rightAscension;

        /**
         * Declination in the J2000 coordinate system
         * Units: degrees
         **/
        double declination;

        /**
         * Position angle. Counted east from north.
         * Units: radians
         **/
        double positionAngle;

        /**
         * Major axis
         * Units: arcsecs
         **/
        double majorAxis;

        /**
         * Minor axis
         * Units: arcsecs
         **/
        double minorAxis;

        /**
         * Flux at 1400 Mhz
         * Units: Jy
         **/
        double i1400;
    };

    /**
     * A sequence of component identifiers
     **/
    ["java:type:java.util.ArrayList<Long>"]
    sequence<long> ComponentIdSeq;

    /**
     * A sequence of Components
     **/
    ["java:type:java.util.ArrayList<askap.interfaces.skymodelservice.Component>"]
    sequence<Component> ComponentSeq;

    /**
     * This exception is thrown when a component id is specified but the component
     * does not exit, but is expected to.
     **/
    exception InvalidComponentIdException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Interface to the Sky Model Service.
     **/
    interface ISkyModelService
    {
        /**
         * Cone search method.
         *
         * The cone search does not directly return a sequence of components, which
         * could potentially be very large. Instead a sequence of component ids is
         * returned. This allows the caller to then call getComponents() for a subset
         * of the full component list if it is too large. The idea here is to allow
         * the client access perhaps to be hidden behind an iterator which allows
         * the client to deal with one a smaller (more manageable) subset of the
         * result set at a time.
         *
         * @param right_ascension   the right ascension of the centre of the
         *                          search area (Units: decimal degrees).
         * @param declination       the declination of the centre of the search
         *                          area (Units: decimal degrees).
         * @param radius            the search radius (Units: decimal degrees).
         * @param fluxLimit         low limit on flux on sources returned all
         *                          returned sources shall have flux >= fluxLimit
         *                          (Units: Jy).
         *
         * @return                  a sequence of component identifiers.
         **/
        ComponentIdSeq coneSearch(double rightAscension, double declination,
            double searchRadius, double fluxLimit);

        /**
         * Obtain a sequence of components. If a component in the componentIds
         * sequence does not exist in the database, it is simply omitted from
         * the result set, this the size of the returned sequence of components
         * may be less than the size of the component id sequence parameter.
         *
         * @param component_ids     a sequence of component identifiers
         *
         * @return                  a sequence of components.
         **/
        ComponentSeq getComponents(ComponentIdSeq componentIds);

        /**
         * Add a sequence of one or more components to the component database.
         * 
         * Note: This is really just here for testing purposes and will likely
         * not form part of the final API. Indeed the merge LSM into GSM usecase
         * will replace this one.
         *
         * @return a sequence of component ids mapping one-to-one with the
         * "components" sequence passed to the function.
         **/
        ComponentIdSeq addComponents(ComponentSeq components);

        /**
         * Remove components from the components table.
         * 
         * Note: This is really just here for testing purposes and will likely
         * not form part of the final API.
         *
         * @param component_ids a sequence of component identifiers which
         *                      reference the components to delete.
         * @throws InvalidComponentIdException  if one or more of the component
         *                                      identifiers does not exist.
         **/
        void removeComponents(ComponentIdSeq componentIds) throws InvalidComponentIdException;
    };

};
};
};

#endif
