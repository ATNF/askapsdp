/*
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
 * 
 * @author Ben Humphreys <ben.humphreys@csiro.au>
 */
package askap.cp.manager;

// Java imports
import java.util.Map;

// ASKAPsoft imports
import Ice.Current;
import org.apache.log4j.Logger;
import askap.interfaces.component.*;

public class AdminInterface extends askap.interfaces.component._IComponentDisp {
	private Ice.Communicator itsComm;

	private Ice.ObjectAdapter itsAdapter;

	private ComponentState itsState;

	private ObsService itsObsService;
	
	private String itsObsServiceName = "CentralProcessorService";

	/** Logger. */
	static Logger logger = Logger.getLogger(AdminInterface.class.getName());

	public AdminInterface(Ice.Communicator ic) {
		super();
		itsComm = ic;
		itsAdapter = null;
		itsObsService = null;
		itsState = ComponentState.LOADED;
	}

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
				itsComm.stringToIdentity("CentralProcessorAdmin"));
		itsAdapter.activate();

		// Block here so main() can block on this
		itsComm.waitForShutdown();
		logger.info("Shutting AdminInterface");
	}

	@Override
	public void startup(Map<String, String> config, Current curr)
			throws TransitionException {
		if (itsState != ComponentState.LOADED) {
			throw new TransitionException("Not in UNLOADED state");
		}

		itsObsService = new ObsService(itsComm);

		// Must transition to standby only once all objects are created
		itsState = ComponentState.STANDBY;

	}

	@Override
	public void shutdown(Current curr) throws TransitionException {
		if (itsState != ComponentState.STANDBY) {
			throw new TransitionException("Not in STANDBY state");
		}

		// Must transition to LOADED before destroying any objects
		itsState = ComponentState.LOADED;

		itsObsService = null;
	}

	@Override
	public void activate(Current curr) throws TransitionException {
		if (itsState != ComponentState.STANDBY) {
			throw new TransitionException("Not in STANDBY state");
		}

		Ice.Object object = itsObsService;
		itsAdapter.add(object,
				itsComm.stringToIdentity(itsObsServiceName));

		// Block until service is actually registered
		while (itsAdapter.find(itsComm.stringToIdentity(itsObsServiceName)) == null) {
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
	public void deactivate(Current curr) throws TransitionException {
		if (itsState != ComponentState.ONLINE) {
			throw new TransitionException("Not in ONLINE state");
		}

		// Must transition to STANDBY before deactivating any services
		itsState = ComponentState.STANDBY;
		
        // Stop the server
        itsAdapter.remove(itsComm.stringToIdentity(itsObsServiceName));

        // Block until service is actually unregistered
        while (itsAdapter.find(itsAdapter.getCommunicator().stringToIdentity(itsObsServiceName)) != null) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
            	// No consequence
            }
        }
	}

	@Override
	public ComponentTestResult[] selfTest(Current curr)
			throws CannotTestException {
		if (itsState != ComponentState.STANDBY) {
			throw new CannotTestException("Not in STANDBY state");
		}

		return new ComponentTestResult[0];
	}

	@Override
	public ComponentState getState(Current curr) {
		// TODO Auto-generated method stub
		return itsState;
	}

}