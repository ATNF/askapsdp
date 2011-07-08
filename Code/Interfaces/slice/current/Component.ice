// @file Component.ice
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

#ifndef ASKAP_COMPONENT_ICE
#define ASKAP_COMPONENT_ICE

#include <CommonTypes.ice>

module askap
{

module interfaces
{

module component
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
     * This exception is thrown by selfTest() because the component
     * is not in the STANDBY state, and hence is not in a state
     * suitable for testing.
     **/
    exception CannotTestException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Valid states for the component.
     **/
    enum ComponentState
    {
        LOADED,
        STANDBY,
        ONLINE
    };

    /**
     * Valid test results.
     **/
    enum ComponentTestStatus
    {
        PASS,
        FAIL
    };

    /**
     * This type represents a single test which is run as part of the
     * selfTest() call.
     **/
    struct ComponentTestResult
    {
        string name;
        ComponentTestStatus result;
        string details;
    };

    /**
     * This is the type via which the summary of test results from the
     * selfTest() call is returned.
     **/
    sequence<ComponentTestResult> ComponentTestResultSeq;

    /**
     * Common interface which each component in the ASKAP online system needs
     * to implement. This is an administrative interface via which the
     * component will be administered. In addition to this administrative
     * interface a component shall provide one ore more services. The interface
     * to these services is component specific and hence is not defined here,
     * however the lifecycle and availability of these services does relate to
     * the component state model and is discussed in the comments below.
     *
     * The behaviour of a component is expected to be as follows:
     * 
     * UNLOADED - When in the unloaded state the component shall provide this
     * administrative interface. i.e. its implementation of IComponent. However
     * its service interfaces shall not be available for calling and the
     * component shall be in a mostly uninitialized state.
     *
     * STANDBY - When in the standby state the component shall provide this
     * administrative interface but would usually also have created its
     * internal objects such that a self test could be performed. Howevever
     * its service interfaces shall not be available for calling.
     *
     * ONLINE - When in the online state the component shall provide this
     * administrative interface and shall also provide all services interfaces
     * for calling.
     *
     * The state transitions can be thought of in the following manner:
     *
     * Startup/Shutdown - These two transitions are essentially responsible
     * for creation and destruction of the services the component provides.
     * 
     * Activate/Deactivate - These two transitions make available/unavailable 
     * the services in the Ice locator service (registry).
     *
     **/
    interface IComponent
    {
        /**
         * Startup the component. That is, transition it from the  LOADED
         * state to STANDBY.
         *
         * While the component is in the transition from UNLOADED to STANDBY,
         * calls to getState() will return UNLOADED.
         *
         * If this call returns without throwning an exception then the state
         * transition completed cleanly.
         *
         * @param  params  configuration parameters which the component
         *          should use.
         *
         * @throws TransitionException Raised if the state transition fails.
         *          A reason shall be specified in the reason field.
         **/
        ["ami"] void startup(askap::interfaces::ParameterMap config)
            throws TransitionException;

        /**
         * Shutdown the component. That is, transition it from the STANDBY
         * state to LOADED.
         *
         * While the component is in the transition from STANDBY to UNLOADED,
         * calls to getState() will return UNLOADED.
         *
         * If this call returns without throwning an exception then the state
         * transition completed cleanly.
         *
         * @throws TransitionException Raised if the state transition fails.
         *          A reason shall be specified in the reason field.
         **/
        ["ami"] void shutdown() throws TransitionException;

        /**
         * Activate the component. That is, transition it from the STANDBY state
         * to ONLINE.
         *
         * When this function returns (without throwing an exception), the 
         * interfaces for all the services the component provides will be 
         * available in the locator service and available for calling. It 
         * is also usual for the component to perform at least some minimal
         * self-testing to ensure at least minimum functionality can be
         * provided.
         *
         * While the component is in the transition from STANDBY to ONLINE,
         * calls to getState() will return STANDBY. Only when all
         * services/interface are available will calls to getState() return
         * ONLINE.
         *
         * If this call returns without throwning an exception then the state
         * transition completed cleanly.
         *
         * @throws TransitionException Raised if the state transition fails.
         *         A reason shall be specified in the reason field.
         **/
        ["ami"] void activate() throws TransitionException;

        /**
         * Deactivate the component. That is, transition it from the ONLINE
         * state to STANDBY.
         *
         * While the component is in the transition from ONLINE to STANDBY,
         * calls to getState() will return STANDBY.
         *
         * If this call returns without throwning an exception then the state
         * transition completed cleanly.
         *
         * @throws TransitionException Raised if the state transition fails.
         *         A reason shall be specified in the reason field.
         **/
        ["ami"] void deactivate() throws TransitionException;

        /**
         * Returns the state of the component.
         *
         * @return  the state of the component.
         **/
        idempotent ComponentState getState();

        /**
         * Returns a string containing the version of the component.
         *
         * @return  the version of the component.
         **/
        ["ami"] idempotent string getVersion();

        /**
         * Requests the component perform a self test. There is no concept
         * of pass/fail for the overall component, this function however
         * returns a sequence containing a list of tests carried out, the
         * results (PASS/FAIL) and a string which may contain some detail,
         * typically reason for failure however it may also contain detail
         * in the case of a passing test also.
         *
         * The selftest can only be run when the component is in the STANDBY
         * state.
         *
         * @throws CannotTestException if this function is called when the
         *         component is in a state other than STANDBY.
         *
         * @return a map containing a name to identify the test and the
         *         test result.
         **/
        ["ami"] ComponentTestResultSeq selfTest() throws CannotTestException;
    };
};
};
};

#endif
