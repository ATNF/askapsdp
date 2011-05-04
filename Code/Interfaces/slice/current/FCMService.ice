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
    
    exception NoSuchKeyException extends askap::interfaces::AskapIceException
    {
    };
    
    exception KeyExistsException extends askap::interfaces::AskapIceException
    {
    };

    enum UpdateStatus {
        ADDED,
        UPDATED,
        REMOVED
    };

    struct RevisionInfo {
        long revision;
        string user;
        string log;
        string date;
    };
    
    sequence<RevisionInfo> RevisionInfoSeq;
    
    struct UpdatedKeys {
        ParameterMap parameters;
        RevisionInfo info;
        UpdateStatus state;        
    };
    
    /**
     * 
     **/
    interface IFCMService
    {
        /**
         * Retrieve the facility configuration parameters at the specified
         * version. A version less than one will return the latest revision.
         * If key is empty all parameters are returned otherwise only those
         * starting with the given key.
         **/
        idempotent ParameterMap get(long revision, string key)
            throws NoSuchKeyException;
        /**
         * Store the given list of configuration parameters. This adds
         * or overwrites. The user name and a log message are also required.
         **/
        long update(ParameterMap parms, string user, string log)
            throws NoSuchKeyException;
        /**
         * Store the given list of configuration parameters. This adds
         * or overwrites. The user name and a log message are also required.
         **/
        long add(ParameterMap parms, string user, string log)
            throws KeyExistsException;

        /**
         * Remove the given keys from the configuration. The user name and a
         * log message are also required.
         **/
        long remove(StringSeq keys, string user, string log)
            throws NoSuchKeyException;
        /**
         * Get the information about the revision history up to the given
         * revision.
         * If key is empty all history entries are returned otherwise
         * only the history for the given key.
         * A revision less than one indicates the whole history
         **/
        idempotent RevisionInfoSeq history(long revision, string key);

    };
    
    /**
     * Publisher FCM change events.
     **/
    interface IFCMMonitor {
        /**
         * Notify when a configuration has changed
         **/
        idempotent void updated(UpdatedKeys changes);
        
    };
};
};
};

#endif
