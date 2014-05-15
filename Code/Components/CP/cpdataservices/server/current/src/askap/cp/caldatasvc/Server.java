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
package askap.cp.caldatasvc;

// System imports
import java.io.File;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.PropertyConfigurator;
import askap.cp.utils.ServiceManager;

public class Server extends Ice.Application {

	/**
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(Server.class.getName());
	
	@Override
	public int run(String[] args) {
		// Init logging
		final String logcfg = "askap.log_cfg";
		File f = new File(logcfg);
		if (f.exists()) {
			PropertyConfigurator.configure(logcfg);
		} else {
			BasicConfigurator.configure();
		}

		logger.info("ASKAP Calibration Data Service (Server)");

		int status = 0;
		try {
			CalibrationDataServiceImpl svc = new CalibrationDataServiceImpl(communicator());

			// Blocks until shutdown
			ServiceManager.runService(communicator(), svc, "CalibrationDataService",
					"CalibrationDataServiceAdapter");
		} catch (Exception e) {
			System.err.println(e.getMessage());
			e.printStackTrace();
			status = 1;
		}
		return status;
	}
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Server svr = new Server();
		int status = svr.main("CalibrationDataService", args);
		System.exit(status);
	}
}
