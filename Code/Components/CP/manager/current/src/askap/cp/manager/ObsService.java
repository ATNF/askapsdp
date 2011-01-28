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
import Ice.Current;
import askap.interfaces.cp._ICPObsServiceDisp;

public class ObsService extends _ICPObsServiceDisp {
	
	private static final long serialVersionUID = 1L;
	
    //private Ice.Communicator itsComm;
	
	/** Logger. */
	private static Logger logger = Logger.getLogger(ObsService.class.getName());
    
	public ObsService(Ice.Communicator ic) {
		//itsComm = ic;
		logger.info("Creating ObsService");
	}
	
	public void finalize() {
		logger.info("Destroying ObsService");
	}
	
	public void abortObs(Current curr) {
		// TODO Auto-generated method stub
	}

	public void startObs(long sbid, Current curr) {
		// TODO Auto-generated method stub
	}

}
