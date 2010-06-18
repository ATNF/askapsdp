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
#include <DataServiceExceptions.ice>

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
     * The SchedulingBlockService provides and interface to the main ASKAP
     * obseravtion entity - the Scheduling Block. The Scheduling Block describes
     * and observation and its execution which is done througth the Executive.
     **/
    interface ISchedulingBlockService
    {
        /**
         * Transition the Scheduling Block to the given state.
         *
         * @param sbid The id of the Scheduling Block
         * @param newstate The state to transition to.
         *
         **/
        void transition(long sbid,
                        ObsState newstate) throws
		TransitionException,
		NoSuchSchedulingBlockException;

        /**
         * Get all Scheduling Block ids matching the given state.
         *
         * @return a sequence of Scheduling Block ids
         *
         **/
        idempotent askap::interfaces::LongSeq getByState(ObsState state);

        /**
         * Create a new Scheduling Block with the given initial configuration.
         * This will have and empty set of ObsVariables and ill be in DRAFT
         * state.
         *
         * @param programid The id of the Owner Observation Program.
         * @param sbtid The id of the Scheduling BlockTemplate to use
         * @return The id of the newly created Scheduling Block
         *
         **/
        long create(long programid, long sbtid)
             throws NoSuchSBTemplateException,
                    ParameterException;

        /**
         * Remove and existing Scheduling Block. This will only work with
         * Scheduling Blocks in DRAFT state otherwise an exception is thrown.
         * @param sbid The id of the Scheduling Block
         **/
        void remove(long sbid) throws TransitionException,
                                      NoSuchSchedulingBlockException;

        /**
         * Get the POSIX time the given Scheduling Block
         * has been scheduled for. If the Scheduling Block isn't in SCHEDULED
         * state return a value less than 0. 
         *
         * @param sbid The id of the Scheduling Block
         * @return an POSIX time double
         *
         **/
        idempotent double getScheduledTime(long sbid)
               throws NoSuchSchedulingBlockException;

        /**
         * Return the actual Observation Parameters which are the result of
         * a merge of the SBTemplate's Obs Parameters and the Obs User
         * Parameters. This is used in the execution of the Scheduling Block.
         * @param sbid The id of the Scheduling Block
         * @return a ParameterMap
         **/
        idempotent askap::interfaces::ParameterMap getObsParameters(long sbid)
                throws NoSuchSchedulingBlockException;


        /**
         * Get the Observation Procedure i.e. the python script
         *
         * @param sbid The id of the Scheduling Block
         * @return a string
         *
         **/
        idempotent string getObsProcedure(long sbid)
            throws NoSuchSchedulingBlockException;

        /**
         * Set the UserParameters to the given values. This replaces the
         * UserParameters and any keys which exist in the Scheduling Block but
         * not in the given userparams will be deleted.
         *
         * @param sbid The id of the Scheduling Block
         * @param userparams The Observation User Parameters to set/overwrite
         *
         **/
        void setObsUserParameters(long sbid,
                                  askap::interfaces::ParameterMap userparams)
                throws ParameterException,
                       NoSuchSchedulingBlockException;


        /**
         * Get the Observation Variables. This allows for
         * returning a subset e.g. for component. If all ObsVariables are to
         * be returned specify and empty string.
         *
         * @param sbid The id of the Scheduling Block
         * @param key The key of the ObsVariable subset to return
         * @return a ParameterMap
         *
         **/
        idempotent askap::interfaces::ParameterMap getObsVariables(long sbid,
                                                                    string key)
                throws NoSuchSchedulingBlockException,
                       ParameterException;

        /**
         * Set the Observation Variables to the given values.
         * This overwrites existing and creates newly specified ObsVariables.
         * Omitted variables remain unmodified.
         * in the Scheduling Block but are not in the given in obsvars will
         * be deleted
         *
         * @param sbid The id of the Scheduling Block
         * @param obsvars the new Obs Variables
         *
         **/
        void setObsVariables(long sbid,
                            askap::interfaces::ParameterMap obsvars)
                throws ParameterException,
                       NoSuchSchedulingBlockException;

        /**
         * Remove the specified keys from the ObsVariables.
         *
         * @param sbid The id of the Scheduling Block
         * @param obsvarkeys The Obs Variables to remove
         *
         **/
        void removeObsVariables(long sbid,
                                 askap::interfaces::StringSeq obsvarkeys)
                throws ParameterException,
                       NoSuchSchedulingBlockException;

        /**
         * Set the Template id.
         * This would typically only be used to bump the version of the
         * Template.
         *
         * @param sbid The id of the Scheduling Block
         **/
        void setSBTemplate(long sbid, long sbtid)
                throws NoSuchSchedulingBlockException,
                       NoSuchSBTemplateException;

        /**
         * Get the ObsProgram which owns the given Scheduling Block.
         *
         * @param sbid The id of the Scheduling Block
         * @returns a list of ObsProgram ids
         *
         **/
        idempotent long getOwner(long sbid);

        /**
         * Get a list of all ObsPrograms associated  with the given Scheduling
         * Block.
         *
         * @param sbid The id of the Scheduling Block
         * @returns a list of ObsProgram ids
         *
         **/
        idempotent askap::interfaces::LongSeq getObsPrograms();

        /**
         * Associate the given ObsProgram with the given Scheduling Block.
         *
         * @param sbid The id of the Scheduling Block
         * @param opid The id of the ObsProgram
         *
         **/
        void addObsProgram(long sbid, long opid);

        /**
         * Disassociate an ObsProgram from the given Scheduling Block.
         *
         * @param sbid The id of the Scheduling Block
         * @param opid The id of the ObsProgram
         *
         **/
        void removeObsProgram(long sbid, long opid);

    };
};
};
};

#endif
