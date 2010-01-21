// @file TOS.ice
//
// @copyright (c) 2009 CSIRO
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

#ifndef ASKAP_TOS_ICE
#define ASKAP_TOS_ICE

#include <CommonTypes.ice>

module askap
{

module interfaces
{

module tos
{  
    /**
     * Exception for when a specified antenna cannot be allocated because it
     * has already been allocated by the TOS Manager.
     **/
    exception AntAlreadyAllocException
      extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Exception for when an operation was requested on an antenna which has
     * not been allocated to this client for control.
     **/
    exception AntNotAllocException
      extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Exception for when a construction of a composite control tree would
     * create an illegal structure. Only trees are allowed, ie where there is
     * exactly one path between any two nodes. Any node may only appear in the
     * tree once (although nodes may exist in multiple trees concurrently).
     **/
    exception IllegalTopologyException
      extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Exception for when an operation was requested on an ArrayComponent
     * with an identifier which is unknown to the server.
     **/
    exception NoSuchComponentException
      extends askap::interfaces::AskapIceException
    {
    };
    
    /**
     * Exception thrown when there was an attempt to register illegal settings,
     * which may include the case that the settings are intrinsically valid but
     * are incompatible with already registered settings for global/shared
     * resources.
     **/
    exception IllegalSettingsException
      extends askap::interfaces::AskapIceException
    {
    };
    
    /**
     * Exception for when a specified timeout is exceeded.
     **/
    exception TimeoutException
      extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Exception for if the requested scan does not exist.
     **/
    exception NoSuchScanException
      extends askap::interfaces::AskapIceException
    {
    };
    


    interface ITOSService
    {        
        /** 
         * Request excusive control of one or more antennas as specified (using
         * TBD semantics) by the supplied parameters. If more than one antenna
         * has been requested then a composite ArrayComponent will be returned
         * which has all of the antennas as direct children.
         **/
        string allocate(string clientid, 
                        askap::interfaces::ParameterMap params)
          throws AntAlreadyAllocException,
                 IllegalSettingsException;
        
        /**
         * Release control of all resources which have been allocated to the 
         * specified client.
         **/
        void deallocate(string clientid);
        
        /**
         * Create a new control composite which has the specified components
         * as children. Returns the unique identifier of the new composite.
         **/
        string makeComposite(string clientid,
                             StringSeq arrayids)
          throws AntNotAllocException,
                 IllegalTopologyException,
                 NoSuchComponentException;
        
        /**
         * Return the identifier strings for all immediate children of the
         * specified component. This will return a zero-length array if the
         * specified component is an antenna/leaf node.
         **/
        StringSeq getChildren(string arrayid)
          throws NoSuchComponentException;

        /**
         * Command the specified component to begin execution of a new scan. If
         * another scan is already in progress then it will be stopped prior to
         * commencing the new scan.
         **/
        void startScan(string clientid,
                       string arrayid,
                       askap::interfaces::ParameterMap params,
                       int scannum)
          throws AntNotAllocException,
                 NoSuchComponentException,
                 NoSuchScanException;

        /**
         * Stop any scans which the specified component, and children thereof,
         * are currently performing.
         **/          
        void stopScan(string clientid,
                      string arrayid)
          throws AntNotAllocException,
                 NoSuchComponentException;
                 
        /**
         * Block until all antennas (below the specified component) are no 
         * longer executing a scan. A TimeoutException will be thrown if the
         * scan has not completed within the specified duration.
         */
        void waitForScan(string arrayid,
                         int timeoutms)
          throws TimeoutException,
                 NoSuchComponentException;
         
        /**
         * Stow the specified component. If the antennas are currently scanning
         * the scan will be stopped.
         **/
        void stow(string clientid,
                  string arrayid)
          throws AntNotAllocException,
                 NoSuchComponentException;
    };
};
};
};

#endif
