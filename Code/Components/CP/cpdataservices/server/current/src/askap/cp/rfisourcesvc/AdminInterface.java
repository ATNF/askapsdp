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
package askap.cp.rfisourcesvc;

// Java imports
import java.util.Map;

// ASKAPsoft imports
import Ice.Current;
import org.apache.log4j.Logger;
import askap.interfaces.component.*;

/**
 * This class implements the askap.interfaces.component.IComponent
 * interface, allowing the sky model service to be administered
 * (i.e. started, shutdown) programmatically.
 */
public class AdminInterface extends askap.interfaces.component._IComponentDisp {
	
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
	 * Reference to the DalibrationDataServiceImpl Service object, which provides
	 * the Calibration Data service.
	 */
	private RFISourceServiceImpl itsService;
	
	/**
	 * Ice identity of the observation service.
	 */
	private String itsServiceName = "";
	
	private String itsAdminName = "";

	/** Logger. */
	private static Logger logger = Logger.getLogger(AdminInterface.class.getName());

	/**
	 * Constructor
	 * @param ic	An already initialised Ice communicator for the object
	 * 				to use.
	 */
	public AdminInterface(Ice.Communicator ic, final String serviceName, final String adminName) {
		super();
		itsServiceName = serviceName;
		itsAdminName = adminName;
		
		logger.debug("Creating AdminInterface");
		itsComm = ic;
		itsAdapter = null;
		itsService = null;
		itsState = ComponentState.LOADED;
	}

    public void finalize() {
		logger.debug("Destroying AdminInterface");
    }

	/**
	 * 
	 */
    @Override
	public synchronized void activate(Current curr) throws TransitionException {
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

	/**
	 * 
	 */
    @Override
	public synchronized void deactivate(Current curr) throws TransitionException {
		if (itsState != ComponentState.ONLINE) {
			throw new TransitionException("Not in ONLINE state");
		}

		// Must transition to STANDBY before deactivating any services
		itsState = ComponentState.STANDBY;
		
        // Stop the server
        itsAdapter.remove(itsComm.stringToIdentity(itsServiceName));

        // Block until service is actually unregistered
        while (itsAdapter.find(itsAdapter.getCommunicator().stringToIdentity(itsServiceName)) != null) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
            	// No consequence
            }
        }
	}

	/**
	 * 
	 */
    @Override
	public ComponentState getState(Current curr) {
		// TODO Auto-generated method stub
		return itsState;
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
		itsAdapter.add(object,
				itsComm.stringToIdentity(itsAdminName));
		itsAdapter.activate();

		// Block here so main() can block on this
		itsComm.waitForShutdown();
		logger.info("Stopping AdminInterface");

        itsAdapter.deactivate();
        itsAdapter.destroy();
        logger.info("AdminInterface stopped");
    }

	/**
	 * 
	 */
	@Override
	public synchronized ComponentTestResult[] selfTest(Current curr)
			throws CannotTestException {
		if (itsState != ComponentState.STANDBY) {
			throw new CannotTestException("Not in STANDBY state");
		}

		return new ComponentTestResult[0];
	}

	/**
	 * 
	 */
	@Override
	public synchronized void shutdown(Current curr) throws TransitionException {
		if (itsState != ComponentState.STANDBY) {
			throw new TransitionException("Not in STANDBY state");
		}

		// Must transition to LOADED before destroying any objects
		itsState = ComponentState.LOADED;

		itsService = null;
		
		// Not critical, but is good if garbage collection can happen here
		System.gc();
	}

	/**
	 * 
	 */
	@Override
	public synchronized void startup(Map<String, String> config, Current curr)
			throws TransitionException {
		if (itsState != ComponentState.LOADED) {
			throw new TransitionException("Not in UNLOADED state");
		}

		itsService = new RFISourceServiceImpl(itsComm);

		// Must transition to standby only once all objects are created
		itsState = ComponentState.STANDBY;
	}
}
