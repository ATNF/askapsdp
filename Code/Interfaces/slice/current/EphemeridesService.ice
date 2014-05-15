// @file EphemeridesService.ice
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

#ifndef ASKAP_EPHEMERIDESSERVICE_ICE
#define ASKAP_EPHEMERIDESSERVICE_ICE

#include <CommonTypes.ice>
#include <IService.ice>

module askap
{

module interfaces
{

module ephem
{

    /**
     * This exceptions is thrown when a malformed query is passed into the
     * method.
     **/
    exception InvalidQueryException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Encapsulation of source query parameters. Note that it is useful to
     * create a default instance in the implementations.
     **/
    struct SourceQuery {
        /**
         *  SQL LIKE match on the source name, e.g. 'PKS' will become '%PKS%'
         *  An empty string means no restriction.
         **/
        string name;

        /**
         * The centre of the "cone search". This needs to be specified as
         * \[RA, Dec] in radians
         * An empty sequence indicate no restictions.
         **/
        FloatSeq centre;

        /**
         * The "cone search" radius in degrees to search around the centre.
         * This is only useful if a centre is given.
         **/
        float radius;

        /**
         * The observing frame e.g. J2000
         **/
        string frame;

        /**
         * The flux/intensity of the source. This can be an arbitrary unit
         **/
        float flux;

        /**
         *  Match on a given source catalogue name.
         *  An empty string means no restriction.
         **/
        string catalogue;
    };

    /**
     * A source catalogue entry representation
     **/
    struct Source {
        /**
         * The common name of the source
         **/
        string name;
        /**
         * The position of the source as \[RA, Dec] in radians
         **/
        FloatSeq position;

        /**
         * The observing frame e.g. J2000
         **/
        string frame;

        /**
         * The flux/intensity of the source. This can be an arbitrary unit
         **/
        float flux;

        /**
         * The cataloge the source entry originated from
         **/
        string catalogue;

        /**
         * Any extra columns which are found in the data. These are not
         * queriable.
         **/
        ParameterMap auxiliary;
    };

    // A sequence of sources
    sequence<Source> SourceSeq;

    /**
     * This interface allows querying the ATCA and Parkes (calibrator) source
     * catalogues.
     **/
    interface ISourceSearch extends askap::interfaces::services::IService
    {

        /**
         * Return a list of source catalogue entries matching the given query.
         * For long lists 'limit' and offset can be applied to sequentially
         * return batches. 1 <= limit <= 1000 and  offset > 0 apply.
         *
         **/
        idempotent SourceSeq query(SourceQuery q, int limit, int offset)
            throws InvalidQueryException;

        /**
         * Return a list names of the catalogues provided by this service.
         **/
        idempotent StringSeq getCatalogues();
    };
};

};

};

#endif
