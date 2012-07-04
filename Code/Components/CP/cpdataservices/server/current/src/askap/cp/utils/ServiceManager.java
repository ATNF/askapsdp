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

// ASKAPsoft imports
import org.apache.log4j.Logger;

public class ServiceManager {
	/**
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(ServiceManager.class
			.getName());

	public static void runService(Ice.Communicator ic, Ice.Object svc,
			final String serviceName, final String adapterName) {

		// Setup an adapter
		Ice.ObjectAdapter adapter = ic.createObjectAdapter(adapterName);
		if (adapter == null) {
			throw new RuntimeException("ICE adapter initialisation failed");
		}

		// Create and register the ObsService object
		adapter.add(svc, ic.stringToIdentity(serviceName));

		// Activate the adapter
		boolean activated = false;
		while (!activated) {
			final int interval = 5; // seconds
			final String baseWarn = "  - will retry in " + interval
					+ " seconds";
			try {
				adapter.activate();
				activated = true;
			} catch (Ice.ConnectionRefusedException e) {
				logger.warn("Connection refused" + baseWarn);
			} catch (Ice.NoEndpointException e) {
				logger.warn("No endpoint exception" + baseWarn);
			} catch (Ice.NotRegisteredException e) {
				logger.warn("Not registered exception" + baseWarn);
			}
			if (!activated) {
				try {
					Thread.sleep(interval * 1000);
				} catch (InterruptedException e) {
					// In this rare case this might happen, faster polling is ok
				}
			}
		}

		// Blocks here
		ic.waitForShutdown();

		logger.info("Stopping ObsService");
		adapter.deactivate();
		adapter.destroy();
		logger.info("ObsService stopped");

	}
}
