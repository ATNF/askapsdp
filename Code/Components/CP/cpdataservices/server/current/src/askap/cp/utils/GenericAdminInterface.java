/**
 *  Copyright (c) 2011 CSIRO - Australia Telescope National Facility (ATNF)
 *  
 *  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 *  PO Box 76, Epping NSW 1710, Australia
 *  atnf-enquiries@csiro.au
 * 
 *  This file is part of the ASKAP software distribution.
 * 
 *  The ASKAP software distribution is free software: you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
package askap.cp.utils;

// Java imports
import java.util.Map;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import Ice.Current;

// Ice Interfaces
import askap.interfaces.component.CannotTestException;
import askap.interfaces.component.ComponentState;
import askap.interfaces.component.ComponentTestResult;
import askap.interfaces.component.TransitionException;
import askap.interfaces.component._IComponentDisp;

/**
 * This class implements the askap.interfaces.component.IComponent
 * interface, allowing a generic service to be administered
 * (i.e. started, shutdown) programmatically.
 */
public class GenericAdminInterface extends _IComponentDisp {

	/**
	 * Id for ISeralizable
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * Ice Communicator
	 */
	private Ice.Communicator itsComm;

	/**
	 * Ice Adapter
	 */
	private Ice.ObjectAdapter itsAdapter;

	/**
	 * Component state
	 */
	private ComponentState itsState;

	/**
	 * Reference to the Service object..
	 */
	private Ice.ObjectImpl itsService;

	/**
	 * Ice identity of the service.
	 */
	private String itsServiceName = "";


	/**
	 * Ice identity of the admin interface.
	 */
	private String itsAdminName = "";

	/**
	 * Logger.
	 * */
	private static Logger logger = Logger.getLogger(
			GenericAdminInterface.class.getName());

	/**
	 * A factory that is used to create the service instance.
	 */
	IServiceFactory itsFactory = null;
	
	/**
	 * Constructor
	 * 
	 * @param ic An already initialised Ice communicator for the object to use.
	 * @param factory A factory that is used to create the service interface.
	 * @param serviceName The ICE identity of the service.
	 * @param adminName The ICE identity of the admin interface.
	 */
	public GenericAdminInterface(Ice.Communicator ic, IServiceFactory factory,
			final String serviceName, final String adminName) {
		super();
		logger.debug("Creating AdminInterface: " + adminName);
		
		itsComm = ic;
		itsFactory = factory;
		itsServiceName = serviceName;
		itsAdminName = adminName;
		
		itsAdapter = null;
		itsService = null;
		itsState = ComponentState.LOADED;
	}

	public void finalize() {
		logger.debug("Destroying AdminInterface: " + itsAdminName);
	}

	@Override
	public synchronized void activate(Current cur) throws TransitionException {
		if (itsState != ComponentState.STANDBY) {
			throw new TransitionException("Not in STANDBY state");
		}

		Ice.Object object = itsService;
		itsAdapter.add(object, itsComm.stringToIdentity(itsServiceName));

		// Block until service is actually registered
		while (itsAdapter.find(itsComm.stringToIdentity(itsServiceName)) == null) {
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				// No consequence
			}
		}

		// Must transition to ONLINE only once all services are activated
		itsState = ComponentState.ONLINE;
	}

	@Override
	public synchronized void deactivate(Current cur) throws TransitionException {
		if (itsState != ComponentState.ONLINE) {
			throw new TransitionException("Not in ONLINE state");
		}

		// Must transition to STANDBY before deactivating any services
		itsState = ComponentState.STANDBY;

		// Stop the server
		itsAdapter.remove(itsComm.stringToIdentity(itsServiceName));

		// Block until service is actually unregistered
		while (itsAdapter.find(itsAdapter.getCommunicator().stringToIdentity(
				itsServiceName)) != null) {
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				// No consequence
			}
		}
	}

	@Override
	public synchronized ComponentState getState(Current cur) {
		return itsState;
	}

	@Override
	public String getVersion(Current cur) {
		Package p = this.getClass().getPackage();
		return p.getImplementationVersion();
	}

	@Override
	public synchronized ComponentTestResult[] selfTest(Current cur)
			throws CannotTestException {
		if (itsState != ComponentState.STANDBY) {
			throw new CannotTestException("Not in STANDBY state");
		}

		return new ComponentTestResult[0];
	}

	@Override
	public synchronized void shutdown(Current cur) throws TransitionException {
		if (itsState != ComponentState.STANDBY) {
			throw new TransitionException("Not in STANDBY state");
		}

		// Must transition to LOADED before destroying any objects
		itsState = ComponentState.LOADED;

		itsService = null;

		// Not critical, but is good if garbage collection can happen here
		System.gc();
	}

	@Override
	public synchronized void startup(Map<String, String> params, Current cur)
			throws TransitionException {
		if (itsState != ComponentState.LOADED) {
			throw new TransitionException("Not in UNLOADED state");
		}

		itsService = itsFactory.create(itsComm);
		
		// Must transition to standby only once all objects are created
		itsState = ComponentState.STANDBY;
	}

	/**
	 * 
	 */
	public void run() {
		logger.debug("Running AdminInterface");
		if (itsComm == null) {
			throw new RuntimeException("ICE Communicator is null");
		}

		itsAdapter = itsComm.createObjectAdapter("AdminAdapter");
		if (itsAdapter == null) {
			throw new RuntimeException("ICE adapter initialisation failed");
		}

		Ice.Object object = this;
		itsAdapter.add(object, itsComm.stringToIdentity(itsAdminName));
		
		boolean activated = false;
		while(!activated) {
			final int interval = 5; // seconds
			final String baseWarn = "  - will retry in " + interval + " seconds";
			try {
				itsAdapter.activate();
				activated = true;
			} catch (Ice.ConnectionRefusedException e) {
				logger.warn("Connection refused" + baseWarn); 
			} catch (Ice.NoEndpointException e) {
				logger.warn("No endpoint exception" + baseWarn);
			} catch (Ice.NotRegisteredException e) {
				logger.warn("Not registered exception" + baseWarn);
			}
			try {
				Thread.sleep(interval * 1000);
			} catch (InterruptedException e) {
				// In this rare case this might happen, faster polling is ok
			}
		}

		// Block here so main() can block on this
		itsComm.waitForShutdown();
		logger.info("Stopping AdminInterface");

		itsAdapter.deactivate();
		itsAdapter.destroy();
		logger.info("AdminInterface stopped");
	}
}
