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
     * Exception for when a construction of a composite control tree was 
     * requested which contains no actual antenna leaf nodes in the tree
     * below it.
     **/
    exception NoLeafAntsException
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
    
    
    
    // Forward declaration
    class IArrayComponent;
    
    // An ArrayComponentArray is a collection of one or more ArrayComponents
    sequence<IArrayComponent> IArrayComponentArray;
    
    /**
     * An ArrayComponent is a node in the antenna control tree. An 
     * ArrayComponent may be an individual antenna (a leaf node) or a 
     * composite which contains one or more children.
     **/
    class IArrayComponent {
        string uniqueid;
        bool leaf;
        IArrayComponentArray children;
    };



    interface ITOSService
    {        
        /** 
         * Request excusive control of one or more antennas as specified (using
         * TBD semantics) by the supplied parameters. If more than one antenna
         * has been requested then a composite ArrayComponent will be returned
         * which has all of the antennas as direct children.
         **/
        IArrayComponent allocate(string clientid, 
                                 askap::interfaces::ParameterMap params)
          throws AntAlreadyAllocException;
        
        /**
         * Release exclusive control of the specified antennas. No further scans
         * or other operations may be undertaken once antennas have been
         * deallocated (apart from reallocating the antennas). The 
         * AntennaComposite tree will be pruned to remove any branches which no
         * longer have any antenna leaf nodes after deallocation of the
         * antennas.
         **/
        void deallocate(string clientid,
                        IArrayComponent ants)
          throws AntNotAllocException,
                 NoSuchComponentException;
        
        /**
         * Create a new control composite which has the specified 
         * ArrayComponents as children. Each child must have at least one
         * actual antenna leaf node somewhere in the tree below them, ie a
         * composite which contains no antennas in the tree is illegal.
         **/
        IArrayComponent makeComposite(string clientid,
                                      IArrayComponentArray ants)
          throws AntNotAllocException,
                 NoLeafAntsException,
                 NoSuchComponentException;

        /**
         * Command the specified antennas to begin execution of a new scan. If
         * another scan is already in progress then it will be aborted prior to
         * commencing the new new scan.
         **/
        void startScan(string clientid,
                       IArrayComponent ants,
                       askap::interfaces::ParameterMap params)
          throws AntNotAllocException,
                 NoSuchComponentException;

        /**
         * Stop any scans which are currently in progress.
         **/          
        void abortScan(string clientid,
                       IArrayComponent ants)
          throws AntNotAllocException,
                 NoSuchComponentException;
        
        /**
         * Stow the specified antennas. If the antennas are currently scanning
         * the scan will be aborted throughout the tree below the specified
         * level. Stowing antennas does not imply deallocation or pruning from
         * the tree.
         **/
        void stow(string clientid,
                  IArrayComponent ants)
          throws AntNotAllocException,
                 NoSuchComponentException;
    };
};
};
};

#endif
