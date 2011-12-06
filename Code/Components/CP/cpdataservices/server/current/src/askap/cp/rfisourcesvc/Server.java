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
package askap.cp.rfisourcesvc;

// System imports
import java.io.File;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.PropertyConfigurator;
import askap.cp.utils.GenericAdminInterface;

public class Server {

	/** Logger. */
	private static Logger logger = Logger.getLogger(Server.class.getName());

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		// Init logging
		final String logcfg = "askap.log_cfg";
		File f = new File(logcfg);
		if (f.exists()) {
			PropertyConfigurator.configure(logcfg);
		} else {
			BasicConfigurator.configure();
		}

		logger.info("ASKAP RFI Source Service (Server)");

		// Init Ice and run the admin interface
		int status = 0;
		Ice.Communicator ic = null;
		try {
			ic = Ice.Util.initialize(args);
			if (ic == null) {
				throw new RuntimeException("ICE Communicator initialisation failed");
			}

			GenericAdminInterface admin = new GenericAdminInterface(ic,
					new RFISourceServiceFactory(),
					"RFISourceService", "RFISourceServiceAdmin");
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
