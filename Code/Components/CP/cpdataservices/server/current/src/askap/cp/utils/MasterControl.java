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
import java.util.TreeMap;

// Slice interfaces
import askap.interfaces.component.IComponentPrx;
import askap.interfaces.component.IComponentPrxHelper;
import askap.interfaces.component.TransitionException;

public class MasterControl {

	enum Transition {
		STARTUP,
		SHUTDOWN,
		ACTIVATE,
		DEACTIVATE
	}
	
	static Transition parseTransition(final String str) {
		if (str.equalsIgnoreCase("startup")) {
			return Transition.STARTUP;
		} else if (str.equalsIgnoreCase("shutdown")) {
			return Transition.SHUTDOWN;
		} else if (str.equalsIgnoreCase("activate")) {
			return Transition.ACTIVATE;
		} else if (str.equalsIgnoreCase("deactivate")) {
			return Transition.DEACTIVATE;
		} else {
			throw new RuntimeException("Invalid state transition: " + str);
		}
	}
	
	/**
	 * Simple utility to control state of Ice components
	 * @param args
	 */
	public static void main(String[] args) {
		if (args.length != 4) {
			System.err.println("usage: MasterControl <locator host> <locator port> " +
					"<service admin interface> <state transition>");
			System.exit(1);
		}

		final String locatorHost = args[0];
		final String locatorPort = args[1];
		final String serviceName = args[2];
		final String stateTransition = args[3];

		Ice.Communicator ic = null;
		IComponentPrx admin = null;
		try {
			Ice.Properties props = Ice.Util.createProperties();
			props.setProperty("Ice.Default.Locator", "IceGrid/Locator:tcp -h "
					+ locatorHost + " -p " + locatorPort);
	
			// Initialize a communicator with these properties.
			Ice.InitializationData id = new Ice.InitializationData();
			id.properties = props;
			ic = Ice.Util.initialize(id);
			
			// Get a proxy to the admin interface
			Ice.ObjectPrx base = ic.stringToProxy(serviceName);
			admin = IComponentPrxHelper.checkedCast(base);
			if (admin == null) {
				System.err.println("Invalid proxy for: " + serviceName);
				System.exit(1);
			}
		
			// Get the initial state
			System.out.println("Initial state: " + admin.getState());
			
			// Perform the state transition
			switch (parseTransition(stateTransition)) {
			case STARTUP:
					admin.startup(new TreeMap<String, String>());
				break;
			case SHUTDOWN:
				admin.shutdown();
				break;
			case ACTIVATE:
				admin.activate();
				break;
			case DEACTIVATE:
				admin.deactivate();
				break;
			default:
				System.err.println("Unhandled target state");
				System.exit(1);
			}
			
		} catch (TransitionException e) {
			System.out.println("State transition error: " + e.reason);
		} catch (Exception e) {
			System.out.println("Exception: " + e);
		} finally {
			// Get the final state
			if (admin != null) {
				System.out.println("Final state: " + admin.getState());
			}
			if (ic != null) {
				ic.destroy();
			}
		}
		
	}

}
