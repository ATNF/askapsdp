// @file FCMService.ice
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

#ifndef ASKAP_FCMSERVICE_ICE
#define ASKAP_FCMSERVICE_ICE

#include <CommonTypes.ice>

module askap
{

module interfaces
{

module fcm
{

    /**
     * 
     **/
    interface FCMService
    {
        /**
         * Retrieve the current (latest) facility configuration parameters.
         * If key is empty all parameters are returned otherwise all keys
         * starting with the given key.
         **/
        idempotent ParameterMap get(string key);

        /**
         * Retrieve facility configuration parameters for the given time (the
         * last change before the given time).
         * If key is empty all parameters are returned otherwise all keys
         * starting with the given key.
         **/
        idempotent ParameterMap getByDate(string key, double posixtime);


        ParameterMap put(ParameterMap parms, string user);
        /**
         * Remove the given keys from the configuration. 
         **/
        void remove(StringSeq keys);

        /**
         * Get the history i.e. a list of timestamps when changes occured.
         **/
        idempotent DoubleSeq history();

        /**
         * A pub/sub method to notify of any calls to put() calls.
         **/
        idempotent updated(ParameterMap parms);
        
    };
};
};
};

#endif
