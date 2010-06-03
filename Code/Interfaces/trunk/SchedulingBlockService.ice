// @file SchedulingBlockService.ice
//
// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_SCHEDULINGBLOCKSERVICE_ICE
#define ASKAP_SCHEDULINGBLOCKSERVICE_ICE

#include <CommonTypes.ice>

module askap
{

module interfaces
{

module schedblock
{

    /**
     * This exception is thrown when a state transition fails. The base
     * class includes a "reason" data member (of type string) which shall
     * be used to indicate why the state transition failed.
     **/
    exception TransitionException extends askap::interfaces::AskapIceException
    {
    };


    /**
     * This exceptions is thrown when a ParameterMap with illegal values is
     * passed into the method.
     **/
    exception ParameterException extends askap::interfaces::AskapIceException
    {
    };

    /** This exceptions is thrown when the given Scheduling Block id doesn't
     * exists.
     **/
    exception NoSuchSchedulingBlockException
        extends askap::interfaces::AskapIceException
    {
    };

    /** This exceptions is thrown when the given Observation Program id doesn't
     * exists.
     **/
    exception NoSuchProgramException
        extends askap::interfaces::AskapIceException
    {
    };

    /** This exceptions is thrown when the given Software Instrument id doesn't
     * exists.
     **/
    exception NoSuchSoftwareInstrumentException
        extends askap::interfaces::AskapIceException
    {
    };

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

    interface ISchedulingBlockService
    {
        /**
         * Transition the Scheduling Block to the given state.
         * @param sbid The id of the Scheduling Block
         * @param newstate The state to transition to.
         **/
        void transition(long sbid,
                        ObsState newstate) throws TransitionException,
                                                  NoSuchSchedulingBlockException;

        /**
         * Get all Scheduling Block ids matching the given state.
         * @return a sequence of Scheduling Block ids
         **/
        LongSeq getByState(ObsState state);

        /**
         * Create a new Scheduling Block with the given initial configuration.
         * This will have and empty set of ObsVariables and ill be in DRAFT
         * state.
         * @param programid The id of the Owner Observation Program.
         * @param siid The id of the Software Instrument to use
         * @param userparams The Obseravtion User Parameters
         * @return The id of the newly created Scheduling Block
         **/
        long create(long programid, long siid)
             throws NoSuchSoftwareInstrumentException,
                    ParameterException;

        /**
         * Remove and existing Scheduling Block. This will only work with
         * Scheduling Blocks in DRAFT state otherwise an exception is thrown.
         * @param sbid The id of the Scheduling Block
         **/
        void remove(long sbid) throws TransitionException,
                                      NoSuchSchedulingBlockException;

        /**
         * Make an identical copy of the given SchedulingBlock giving it a new
         * id.
         * @param sbid The id of the Scheduling Block
         * @return The id of the newly created Scheduling Block
         **/
        long clone(long sbid) throws NoSuchSchedulingBlockException;

        /**
         * Get the UTC date/time the given Scheduling Block
         * has been scheduled for.
         * @param sbid The id of the Scheduling Block
         * @return an ISO 8601 UTC date string
         **/
        string getScheduledTime(long sbid)
               throws NoSuchSchedulingBlockException;

        /**
         * Return the actual Observation Parameters which are the result of
         * a merge of the Software Instrument template and the Observation User
         * Parameters. This is used in the execution of the Scheduling Block.
         * @param sbid The id of the Scheduling Block
         * @return a ParameterMap
         **/
        askap::interfaces::ParameterMap getObsParameters(long sbid)
                throws NoSuchSchedulingBlockException;

        /**
         * get the Observation Variables given an optional entry point
         * for a component.
         * @param sbid The id of the Scheduling Block
         * @param nodekey The optional key of the ParameterMap subset to return
         * @return a ParameterMap
         **/
        askap::interfaces::ParameterMap getObsVariables(long sbid,
                                                        string nodekey)
                throws NoSuchSchedulingBlockException,
                       ParameterException;

        /**
         * Get the Observation Procedure
         * @param sbid The id of the Scheduling Block
         * @return a string containing the python script
         **/
        string getObsProcedure(long sbid) throws NoSuchSchedulingBlockException;

        /**
         * Update the UserParameters to the given values.
         * @param sbid The id of the Scheduling Block
         * @param userparams The Obseravtion User Parameters to set/overwrite
         **/
        void setObsUserParameters(long sbid,
                                  askap::interfaces::ParameterMap userparams)
                throws ParameterException,
                       NoSuchSchedulingBlockException;

        /**
         * Update the Observation Variables to the given values.
         **/
        void setObsVariables(long sbid,
                            askap::interfaces::ParameterMap uservars)
                throws ParameterException,
                       NoSuchSchedulingBlockException;

        /**
         * Set the Software Instrument id.
         * This would typically only be used to bump the version of the Software
         * Instrument.
         **/
        void setSoftwareInstrument(long sbid, long siid)
                throws NoSuchSchedulingBlockException,
                       NoSuchSoftwareInstrumentException;
    };
};
};
};

#endif
