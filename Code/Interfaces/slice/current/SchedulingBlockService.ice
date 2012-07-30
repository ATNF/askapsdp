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
#include <SBTemplateService.ice>
#include <IService.ice>

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
     * A sequence of ObsStates.
     *
     **/

    sequence<ObsState> ObsStateSeq;

    /**
     * The SchedulingBlockService provides and interface to the main ASKAP
     * obseravtion entity - the Scheduling Block. The Scheduling Block describes
     * and observation and its execution which is done througth the Executive.
     * A Scheduling Block is associate with a SchedulingBlock Template.
     * This template is versioned. The Scheduling Block always operates on the
     * latest minor version of the template and is not compatible with major
     * version changes unless it is explicitly changed.
     *
     * The Executive has to ensure that it writes into the ObsVariables the
     * actual version it was using to ensures tracability. This is done
     * via the key 'executive.sbtemplate.version'.
     **/
    interface ISchedulingBlockService extends askap::interfaces::services::IService
    {
        /**
         * Transition the Scheduling Block to the given state.
         *
         * @param sbid The id of the Scheduling Block
         * @param newstate The state to transition to.
         *
         **/
        void transition(long sbid, ObsState newstate) throws
		TransitionException,
		NoSuchSchedulingBlockException;

        /**
         * Get all Scheduling Block ids matching the given state.
         *
         * @return a sequence of Scheduling Block ids
         *
         **/
        idempotent askap::interfaces::LongSeq getByState(ObsStateSeq states);

        /**
         * Get all Scheduling Blocks for a given SBTemplate (name) given
         * a version. A value < 0 for majorversion returns all versions.
         *
         * @param name the name of the SBTemplate id
         * @param majorversion the SBTemplate compatible version
         * @return a sequence of Scheduling Block ids
         *
         **/
        idempotent askap::interfaces::LongSeq getByTemplate(string name,
                                                            int majorversion);

	/**
	 * Get Scheduling Block (id) for the ObsProgram "name"
         * @param name the name of the ObsProgram
	 * @return a sequence of Scheduling Block ids
	 *
	 **/	 
	idempotent askap::interfaces::LongSeq getByObsProgram(string name) 
	    throws NoSuchObsProgramException;

        /**
         * Create a new Scheduling Block with the given initial configuration.
         * This will have and empty set of ObsVariables and will be in DRAFT
         * state.
         *
         * @param program The name of the Owner Observation Program.
         * @param templname The name of the Scheduling Block Template to use
         * @param alias a string alias (does not have to be unique)
         * @returns The id of the newly created Scheduling Block
         *
         **/
        long create(string program, string templname, string alias)
             throws NoSuchSBTemplateException,
	            NoSuchObsProgramException;

        /**
         * Remove and existing Scheduling Block. This will only work with
         * Scheduling Blocks in DRAFT state otherwise an exception is thrown.
         *
         * @param sbid The id of the Scheduling Block
         *
         **/
        void remove(long sbid) throws TransitionException,
                                      NoSuchSchedulingBlockException;

        /**
         * Get the alias for the given Scheduling Block
         *
         *  @param sbid The id of the Scheduling Block
         *  @return the string alias
         *
         **/
        string getAlias(long sbid)
               throws NoSuchSchedulingBlockException;

        /**
         * Get the SBTemplate major version (compatible version)
         *  used for the given Scheduling Block.
         *
         *  @param sbid The id of the Scheduling Block
         *  @return the major version number
         *
         **/
        int getVersion(long sbid)
               throws NoSuchSchedulingBlockException;

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
         * Get the Observation Procedure i.e. the python script. This is just
         * forwarded from the SB Template which stores it.
         *
         * @param sbid The id of the Scheduling Block
         * @return a string
         *
         **/
        idempotent string getObsProcedure(long sbid)
            throws NoSuchSchedulingBlockException;

        /**
         * Return the Observation User Parameters which have been defined for
         * this Scheduling Block.
         *
         * @param sbid The id of the Scheduling Block
         * @return a ParameterMap
         *
         **/
        idempotent askap::interfaces::ParameterMap getObsUserParameters(long sbid)
                throws NoSuchSchedulingBlockException;

        /**
         * Set the UserParameters to the given values. This replaces the
         * UserParameters. Any keys which exist in the Scheduling Block but
         * not in the given userparams will be deleted.
         *
         * @param sbid The id of the Scheduling Block
         * @param userparams The Observation User Parameters to set/overwrite
         *
         **/
        void setObsUserParameters(long sbid,
                                  askap::interfaces::ParameterMap userparams)
                throws ParameterException,
                       TransitionException,
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
         * Get the SBTemplate linked to the SchedulingBlock.
         *
         * @param sbid The id of the Scheduling Block
         * @returns a an SBTemplate name
         *
         **/
        idempotent string getSBTemplate(long sbid)
                throws NoSuchSchedulingBlockException;

        /**
         * Set the Template name
         * This would typically only be used to upgrade to a new major version
         *  of the SB Template.
         *
         * @param sbid The id of the Scheduling Block
         * @param tmplname The name of the SB Template
         * @param majorversion the new major version of the template to use
         * @param userparams the new user parameters corresponding to this
         *                   version
         *
         **/
        void updateTemplateVersion(long sbid, int majorversion,
				   askap::interfaces::ParameterMap userparams)
                throws NoSuchSchedulingBlockException,
                       NoSuchSBTemplateException;

        /**
         * Get the State of the given Scheduling Block.
         *
         * @param sbid The id of the Scheduling Block
         * @returns a ObsState enum value
         *
         **/
        idempotent ObsState getState(long sbid);

        /**
         * Get the ObsProgram which owns the given Scheduling Block.
         *
         * @param sbid The id of the Scheduling Block
         * @returns the name of the ObsProgram
         *
         **/
        idempotent string getOwner(long sbid);

        /**
         * Get a list of all ObsPrograms associated  with the given Scheduling
         * Block.
         *
         * @param sbid The id of the Scheduling Block
         * @returns a list of ObsProgram names
         *
         **/
        idempotent askap::interfaces::StringSeq getObsPrograms(long sbid);

        /**
         * Associate the given ObsProgram with the given Scheduling Block.
         *
         * @param sbid The id of the Scheduling Block
         * @param program The id of the ObsProgram
         *
         **/
        void addObsProgram(long sbid, string program);

        /**
         * Disassociate an ObsProgram from the given Scheduling Block.
         *
         * @param sbid The id of the Scheduling Block
         * @param progra, The id of the ObsProgram
         *
         **/
        void removeObsProgram(long sbid, string program);

    };
};
};
};

#endif
