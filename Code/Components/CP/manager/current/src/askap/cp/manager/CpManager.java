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

public class CpManager {

	/**
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(CpManager.class.getName());

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
	private static Ice.Properties getIceProperties(ParameterSet parset) throws IOException {
		Ice.Properties props = Ice.Util.createProperties();
		ParameterSet subset = parset.subset("ice_properties.");
		for (Enumeration e = subset.keys(); e.hasMoreElements(); /**/) {
			String key = (String) e.nextElement();
			String value = subset.getProperty(key);
			props.setProperty(key, value);
		}
		return props;
	}

	/**
	 * Main
	 * @param args
	 */
	public static void main(String[] args) {
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

		// Init Ice and run the admin interface
		int status = 0;
		Ice.Communicator ic = null;
		try {
			ParameterSet parset = new ParameterSet(args[0]);
			Ice.InitializationData id = new Ice.InitializationData();
			id.properties = getIceProperties(parset);
			ic = Ice.Util.initialize(id);

			if (ic == null) {
				throw new RuntimeException("Error: ICE Communicator initialisation failed");
			}

			AdminInterface admin = new AdminInterface(ic, parset);
			admin.run(); // Blocks until shutdown
		} catch (Ice.LocalException e) {
			e.printStackTrace();
			status = 1;
		} catch (Exception e) {
			System.err.println(e.getMessage());
			status = 1;
		}

		if (ic != null) {
			// Cleanup
			try {
				ic.destroy();
			} catch (Exception e) {
				System.err.println(e.getMessage());
				status = 1;
			}
		}
		System.exit(status);
	}
}
