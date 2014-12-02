/**
 *  Copyright (c) 2011-2014 CSIRO - Australia Telescope National Facility (ATNF)
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
package askap.util;

// ASKAPsoft imports
import org.apache.log4j.Logger;

/**
 * This class encapsulates much of the management of an Ice service. Both coarse
 * (blocking) and fine grained (blocking or non-blocking) functionality is
 * provided.
 * 
 * A client wishing to use this in main(), to handle setup/blocking/destruction
 * can simply call runService(). However for more fine grained control, start(),
 * stop() & waitForShutdown() methods are provided.
 */
public class ServiceManager {
	/** Logger */
	private static Logger logger = Logger.getLogger(ServiceManager.class
			.getName());

	/**
	 * Ice adapter
	 */
	Ice.ObjectAdapter adapter;
	
	/**
	 * Ice communicator
	 */
	Ice.Communicator comm;
	
	/**
	 * The name of the service
	 */
	String serviceName = "";

	/**
	 * Starts a service.
	 * This method performs the following:
	 * <ul>
	 * <li> Creates an adapter given the parameter "adapterName"
	 * <li> Registers the service object
	 * <li> Activates the adapter
	 * </ul>
	 * @param ic
	 * @param svc
	 * @param serviceName
	 * @param adapterName
	 */
	public void start(Ice.Communicator ic, Ice.Object svc,
			final String serviceName, final String adapterName) {
		this.comm = ic;
		this.serviceName = serviceName;
		
		// Create an adapter
		adapter = ic.createObjectAdapter(adapterName);
		if (adapter == null) {
			throw new RuntimeException("ICE adapter initialisation failed");
		}

		// Register the service object
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
			} catch (Ice.ConnectFailedException e) {
				logger.warn("Connect failed exception" + baseWarn);
			} catch (Ice.DNSException e) {
				logger.warn("DNS exception" + baseWarn);
			} catch (Ice.SocketException e) {
				logger.warn("Socket exception" + baseWarn);
			}
			if (!activated) {
				try {
					Thread.sleep(interval * 1000);
				} catch (InterruptedException e) {
					// In this rare case this might happen, faster polling is ok
				}
			}
		}
	}

	/**
	 * Block until shutdown has been indicated via the Ice communicator
	 */
	public void waitForShutdown() {
		comm.waitForShutdown();
	}
	
	/**
	 * Deactivates then destroys the Ice adapter
	 */
	public void stop() {
		logger.info("Stopping " + serviceName);
		adapter.deactivate();
		adapter.destroy();
		logger.info(serviceName + " stopped");
	}

	/**
	 * Runs an Ice Service. This call blocks until the Ice communicator is
	 * shutdown.
	 * 
	 * @param ic			the Ice communicator upon which the service will
	 * 						be run
	 * @param svc			the service object
	 * @param serviceName	the name of the service
	 * @param adapterName	the key used to lookup the adapter configuration
	 * 						in the Ice properties
	 */
	public static void runService(Ice.Communicator ic, Ice.Object svc,
			final String serviceName, final String adapterName) {
		ServiceManager manager = new ServiceManager();
		manager.start(ic, svc, serviceName, adapterName);
		manager.waitForShutdown();
		manager.stop();
	}
}
