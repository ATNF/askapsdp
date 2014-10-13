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
 * 
 * @author Ben Humphreys <ben.humphreys@csiro.au>
 */
package askap.cp.manager;

import org.apache.log4j.Logger;

import askap.cp.manager.monitoring.MonitoringSingleton;
import askap.util.ServiceApplication;
import askap.util.ServiceManager;

public class CpManager extends ServiceApplication {
	/** Logger */
	private static Logger logger = Logger.getLogger(CpManager.class.getName());

	/**
	 * @see askap.cp.manager.ServiceApplication#run(java.lang.String[])
	 */
	@Override
	public int run(String[] args) {
		try {
			logger.info("ASKAP Central Processor Manager");

			final String serviceName = config().getString("ice.servicename");
			if (serviceName == null) {
				logger.error("Parameter 'ice.servicename' not found");
				return 1;
			}
			final String adapterName = config().getString("ice.adaptername");
			if (adapterName == null) {
				logger.error("Parameter 'ice.adaptername' not found");
				return 1;
			}

			// Create and register the ObsService object
			ObsService svc = new ObsService(communicator(), config());
			
			// Initialise monitoring interface if configured
			boolean monitoring = config().getBoolean("monitoring.enabled", false);
			if (monitoring) {
				boolean status = initMonitoring();
				if (!status) {
					logger.error("Monitoring sub-system failed to initialise correctly");
				}
			}

			// Blocks until shutdown
			ServiceManager.runService(communicator(), svc, serviceName, adapterName);
			if (monitoring) {
				MonitoringSingleton.destroy();
			}
		} catch (Exception e) {
			logger.error("Unexpected exception: " + e);
		}

		return 0;
	}

	/**
	 * Main
	 * @param args command line arguments
	 */
	public static void main(String[] args) {
		CpManager svr = new CpManager();
		int status = svr.servicemain(args);
		System.exit(status);
	}

	/**
	 * Initialise the monitoring singleton.
	 * @return true if the monitoring sub-system was correctly initialised,
	 * otherwise false.
	 */
	private boolean initMonitoring() {
		final String key1 = "monitoring.ice.servicename";
		final String key2 = "monitoring.ice.adaptername";
		String serviceName = config().getString(key1);
		if (serviceName == null) {
			logger.error("Parameter '" + key1 + "' not found");
			return false;
		}
		String adapterName = config().getString(key2);
		if (adapterName == null) {
			logger.error("Parameter '" + key2 + "' not found");
			return false;
		}
		MonitoringSingleton.init(communicator(),
				serviceName, adapterName);
		return true;
	}
}
