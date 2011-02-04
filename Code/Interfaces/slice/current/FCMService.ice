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
    
    enum UpdateStatus {
        ADDED,
        CHANGED,
        DELETED
    }
    
    struct UpdatedKeys {
        string key;
        UpdateStatus state;
        long revision;
    }

    sequence<UpdatedKeys> UpdatedKeysSeq;
    
    struct History {
        long revision;
        string user;
        string log;
    }
    
    sequence<History> HistorySeq;

    /**
     * 
     **/
    interface FCMService
    {
        /**
         * Retrieve the facility configuration parameters at the specified
         * version. A version less than zero will return the latest revision.
         * If key is empty all parameters are returned otherwise only those
         * starting with the given key.
         **/
        idempotent ParameterMap get(string key, long revision);

        /**
         * Store the given list of configuration parameters. This adds
         * or overwrites. The user name and a log message are also required.
         **/
        long put(ParameterMap parms, string user, string log);

        /**
         * Remove the given keys from the configuration. The user name and a
         * log message are also required.
         **/
        long remove(StringSeq keys, string user, string log);

        /**
         * Get the information about the revision history up to the given
         * revision.
         * A revision less than zero indicates the whole history
         **/
        idempotent HistorySeq history(long revision);

        /**
         * Return the differences between revision1 and revision2.
         **/
        UpdatedKeysSeq diff(long revision1, long revision2);


        /**
         * A pub/sub method to notify of any calls to put() calls.
         **/
        idempotent updated(UpdatedKeysSeq changes);
        
    };
};
};
};

#endif
