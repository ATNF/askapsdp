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

public class CpManager extends ServiceApplication {
	/** Logger */
	private static Logger logger = Logger.getLogger(CpManager.class.getName());

	public CpManager() {
		super();
	}

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

			// Blocks until shutdown
			ServiceManager.runService(communicator(), svc, serviceName, adapterName);
		} catch (Exception e) {
			logger.error("Unexpected exception: " + e.getMessage());
			logger.error(e.getStackTrace());
		}

		return 0;
	}

	/**
	 * Main
	 * @param args
	 */
	public static void main(String[] args) {
		CpManager svr = new CpManager();
		int status = svr.servicemain(args);
		System.exit(status);
	}
}
