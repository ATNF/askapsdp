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

// System imports
import java.io.File;
import java.io.IOException;
import java.util.Enumeration;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.PropertyConfigurator;
import askap.util.ParameterSet;

public class CpManager extends Ice.Application {
	/**
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(CpManager.class.getName());
	
	/**
	 * Configuration parameters from config file
	 */
	private ParameterSet itsParset;

	/**
	 * Builds Ice.Properties from a parset file.
	 * 
	 * @param filename 	the filename of the file containing the parset.
	 * @return 	an Ice.Properties instance with the Ice properties from the
	 * 			parset.
	 * @throws IOException 	if the file at the specified location cannot be 
	 * 						accessed.
	 */
	@SuppressWarnings("rawtypes")
	private static Ice.Properties getIceProperties(ParameterSet parset) {
		Ice.Properties props = Ice.Util.createProperties();
		ParameterSet subset = parset.subset("ice_properties.");
		for (Enumeration e = subset.keys(); e.hasMoreElements(); /**/) {
			String key = (String) e.nextElement();
			String value = subset.getProperty(key);
			props.setProperty(key, value);
		}
		return props;
	}
	
	public CpManager(ParameterSet parset) {
		super();
		itsParset = parset;
	}

	@Override
	public int run(String[] args) {
		if (args.length != 1) {
			System.err.println("Error: Invalid command line arguments");
			System.exit(1);
		}

		// Init logging
		String logcfg = System.getenv("LOGCFG");
		System.err.println("LOGCFG: " + logcfg);
		if (logcfg == null) {
			logcfg = "askap.log_cfg";
		}
		File f = new File(logcfg);
		if (f.exists()) {
			PropertyConfigurator.configure(logcfg);
		} else {
			BasicConfigurator.configure();
		}

		logger.info("ASKAP Central Processor Manager");

		try {

			// Setup an adapter
			Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CentralProcessorAdapter");
			if (adapter == null) {
				throw new RuntimeException("ICE adapter initialisation failed");
			}

			// Create and register the ObsService object
			ObsService svc = new ObsService(communicator(), itsParset);
			adapter.add(svc, communicator().stringToIdentity("CentralProcessorService"));

			// Activate the adapter
			boolean activated = false;
			while(!activated) {
				final int interval = 5; // seconds
				final String baseWarn = "  - will retry in " + interval + " seconds";
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
			communicator().waitForShutdown();

			logger.info("Stopping ObsService");
			adapter.deactivate();
			adapter.destroy();
			logger.info("ObsService stopped");
		} catch (Exception e) {
			System.err.println(e.getMessage());
			e.printStackTrace();
			return 1;
		}

		return 0;
	}


	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// Create initialisation data
		ParameterSet parset = null;
		try {
			parset = new ParameterSet(args[0]);
		} catch (IOException e) {
			System.err.println("Error: Could not open file: " + args[0]);
			System.exit(1);
		}
		Ice.InitializationData id = new Ice.InitializationData();
		id.properties = getIceProperties(parset);

		CpManager svr = new CpManager(parset);
		int status = svr.main("CentralProcessorService", args, id);
		System.exit(status);
	}
}
