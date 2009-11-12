// @file SchedulingBlock.ice
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

#ifndef ASKAP_SCHEDULINGBLOCK_ICE
#define ASKAP_SCHEDULINGBLOCK_ICE

#include <CommonTypes.ice>

module askap
{

module interfaces
{

module schedblock
{
    /**
     * Enumeration of states for SchedulingBlock
     **/
    enum ObsState
    {
        DRAFT,
        SUBMITTED,
        SCHEDULED,
        EXECUTING,
        POSTPROCESSING,
        PENDINGTRANSFER,
        COMPLETED,
        ERRORED
    };

    /**
     * Structure to represent the observing program
     **/
    struct ObsProgram
    {
        /**
         * Uniquely identifies the program
         **/
        long id;

        /**
         * Relationship back to the OPAL project code
         **/
        string project;

        /**
         * Name of the principle investigator for the project
         **/
        string investigator;

        /**
         * Common name of the project.
         **/
        string name;
    };

    sequence<long> LongSeq;

    /**
     * Scheduling Block
     *
     * This structure defines a scheduling block as it is sent/received via
     * ICE. It is not expected components will use this structure for their
     * internal representation of a scheduling block. Instead an object should
     * encapsulate the scheduling block, where the object can be constructed
     * using this simple struct type.
     **/
    struct SchedulingBlock
    {
        /**
         * Uniquely identifies the scheduling block
         **/
        long id;

        /**
         * State of the scheduling block
         **/
        ObsState state;

        /**
         * Obsering program
         **/
        string program;

        /**
         * Observation parameters
         **/
        askap::interfaces::ParameterMap params;

        /**
         * Observation variables
         **/
        askap::interfaces::ParameterMap variables;

        /**
         * Observation procedure
         **/
        string procedure;

        /**
         * List of scheduling block IDs for scheduling block which must be
         * executed before this one
         **/
        LongSeq executeAfter;
    };

};
};
};

#endif
