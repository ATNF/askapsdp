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

// ASKAPsoft imports
import org.apache.log4j.Logger;
import org.apache.log4j.BasicConfigurator;


public class CpManager {

	 /** Logger. */
	private static Logger logger = Logger.getLogger(CpManager.class.getName());
    
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// Set up a simple configuration that logs on the console.
	     BasicConfigurator.configure();
	     
	     logger.info("ASKAP Central Processor Manager");
	     
	     int status = 0;
	     Ice.Communicator ic = null;
	     try {
	    	 ic = Ice.Util.initialize(args);
	            if (ic == null) {
	                throw new RuntimeException("ICE Communicator initialisation failed");
	            }

	            AdminInterface admin = new AdminInterface(ic);
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
