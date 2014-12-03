// @file MonitoringProvider.ice
//
// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_MONITORING_PROVIDER_ICE
#define ASKAP_MONITORING_PROVIDER_ICE

#include <TypedValues.ice>

module askap
{
module interfaces
{
module monitoring
{
    /**
     * Monitoring point status values
     */
    enum PointStatus {
        /**
         * The monitoring point data is valid and no alarm condition is present
         */
        OK,

        /**
         * The monitoring point data is invalid
         */
        INVALID,

        /**
         * The monitoring point data is valid and a MINOR alarm condition
         * is present
         */
        MINORALARM,

        /**
         * The monitoring point data is valid and a MAJOR alarm condition
         * is present
         */
        MAJORALARM
    };

    /**
     * Defines a single monitoring point value
     */
    struct MonitorPoint {
        /**
         * The name of the monitoring point. E.g. "wind.direction"
         */
        string        name;

        /**
         * Absolute time expressed as microseconds since MJD=0.
         */
        long          timestamp;

        /**
         * Contains the value of the monitoring point
         */
        TypedValue     value;

        /**
         * The monitoring point status
         */
        PointStatus   status;

        /**
         * A string containint the unit, or an empty string if unit
         * is not applicable
         */
        string        unit;
    };

    /**
     * Sequence of monitoring point values
     */
    sequence<MonitorPoint> MonitorPointSeq;

    /**
     * Producers of monitoring data implement this interface.
     * The caller provides zero or more point names and the
     * return value will contain at most the same number of
     * monitoring points in the returned sequence.
     *
     * Where a point name is not available it will simply not
     * be included in the result sequence.
     *
     * If an empty sequence is passed, the returned sequence will
     * be empty.
     */
    interface MonitoringProvider {
        idempotent MonitorPointSeq get(StringSeq pointnames);
    };

};
};
};

#endif
