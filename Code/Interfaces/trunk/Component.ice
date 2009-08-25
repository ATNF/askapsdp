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

module askap
{

module interfaces
{

module component
{
    /**
     * Base exception from which the StartupException and the
     * ShutdownException derive from. The reason string shall be
     * used to indicate why the state transition failed.
     **/
    exception TransitionException
    {
        string reason;
    };

    /**
     * An exception to be thrown by the startup() function.
     * The reason string shall be used to indicate why the startup
     * failed.
     **/
    exception StartupException extends TransitionException {};

    /**
     * An exception to be thrown by the shutdown() function.
     * The reason string shall be used to indicate why the startup
     * failed.
     **/
    exception ShutdownException extends TransitionException {};

    /**
     * An exception to be thrown by selfTest() because the component
     * is not in the offline state.
     **/
    exception CannotTestException {};

    /**
     * Valid states for the component.
     **/
    enum ComponentState
    {  
        ONLINE,
        OFFLINE
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
     * This is the type via which the summary of test results from the selfTest()
     * call is returned. The key is a string which uniquely identifies the
     * component, while the value is the test result.
     **/
    dictionary<string, ComponentTestStatus> ComponentTestResults;

    /**
     * This is a common type which can be used as the Ice equivilant of the 
     * LOFAR ParameterSet. This contains a map of variable names and maps
     * them to their value.
     **/
    dictionary<string, string> ParameterMap;

    /**
     * Common interface which each component in the ASKAP online system needs
     * to implement. This is an administrative interface via which the
     * component will be administered.
     *
     * When a component is loaded it shall provide this administrative
     * interface. At startup the actual service interfaces will typically be
     * unavailable and can be started and stopped by this administrative
     * interface.
     **/
    interface IComponent
    {
        /**
         * Startup the component. That is, transition it from the OFFLINE state
         * to ONLINE.
         *
         * When this function returns (without throwing an exception), the 
         * interfaces for all the services the component provides will be 
         * available in the locator service and available for calling. It 
         * is also usual for the component to perform at least some minimal
         * self-testing to ensure at least minimum functionality can be
         * provided.
         *
         * While the component is in the transition between offline and online,
         * calls to getState() will return OFFLINE. Only when ALL
         * services/interface are available will calls to getState() return
         * ONLINE.
         *
         * If ALL services/interfaces cannot be started the component shall:
         * - shutdown those services that did manage to start,
         * - remain in the offline state,
         * - and thrown an exception back to the caller of startup
         * 
         * @throws  StartupException    Raised if the startup fails. A reason
         *          shall be specified in the reason field.
         **/
        ["ami"] void startup(ParameterMap params) throws StartupException;

        /**
         * Shutdown the component. That is, transition it from the ONLINE
         * state to OFFLINE.
         *
         * While the component is in the transition between online and offline
         * calls to getState() will return OFFLINE.
         * 
         * If the shutdown does fail to shutdown ANY of the services/interfaces
         * cleanly then the entire component is considered to be in an
         * inconsistent state and it will exit entirely. The service may then
         * be restarted either manually or automatically, but either way will
         * be unavailable for some period of time.
         *
         * If this call returns without throwning an exception then the state
         * transition completed cleanly.
         *
         * @throws  ShutdownException    Raised if the shutdown fails. A reason
         *          shall be specified in the reason field.
         **/
        ["ami"] void shutdown() throws ShutdownException;

        /**
         * Returns the state of the component.
         *
         * @return  the state of the component.
         **/
        idempotent ComponentState getState();

        /**
         * Requests the component perform a self test. There is no concept
         * of pass/fail for the overall component, this function however
         * returns a map containing a list of tests carried out and the
         * results.
         *
         * The selftest can only be run when the component is in offline
         * mode.
         *
         * @throws  CannotTestException if this function is called when the
         *          component is in a state other than offline.
         *
         * @return  a map containing a name to identify the test and the
         *          test result.
         **/
        ["ami"] ComponentTestResults selfTest() throws CannotTestException;
    };
};
};
};

#endif
