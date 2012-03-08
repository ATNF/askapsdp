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

// Core Java imports
import java.util.Map;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import Ice.Current;
import askap.interfaces.cp._ICPObsServiceDisp;

// Local package includes
import askap.cp.manager.ingest.IngestControl;
import askap.cp.manager.rman.QResourceManager;
import askap.cp.manager.svcclients.DataServiceClient;
import askap.cp.manager.svcclients.FCMClient;


public class ObsService extends _ICPObsServiceDisp {
	
	private static final long serialVersionUID = 1L;
		
	/**
	 * Logger.
	 */
	private static Logger logger = Logger.getLogger(ObsService.class.getName());
	
	/**
	 * Facility Configuration Manager client wrapper instance
	 */
	FCMClient itsFCM;
	
	/**
	 * TOS Data Service client wrapper instance
	 */
	DataServiceClient itsDataService;
	
	/**
	 * Ingest Pipeline Controller
	 */
	IngestControl itsIngestControl;
    
	public ObsService(Ice.Communicator ic) {
		logger.info("Creating ObsService");
		//itsFCM = new FCMClient(ic);
		//itsDataService = new DataServiceClient(ic);
		itsIngestControl = new IngestControl(new QResourceManager());
	}
	
	public void finalize() {
		logger.info("Destroying ObsService");
	}
	
	@Override
	public void startObs(long sbid, Current curr)
			throws askap.interfaces.cp.NoSuchSchedulingBlockException,
			askap.interfaces.cp.AlreadyRunningException
	{
		logger.debug("Querying FCM");
		Map<String, String> fc = itsFCM.get();
		
		logger.debug("Getting observation parameters");
		Map<String, String> obsParams;
		
		try {
			obsParams = itsDataService.getObsParameters(sbid);
		} catch (askap.interfaces.schedblock.NoSuchSchedulingBlockException e) {
			String msg = "Scheduling block " + sbid + " does not exist";
			throw new askap.interfaces.cp.NoSuchSchedulingBlockException(msg);
		}
		
		// Blocking (until started)
		itsIngestControl.start(fc, obsParams);
	}
	
	@Override
	public void abortObs(Current curr) {
		// Blocking (until aborted)
		itsIngestControl.abort();
	}
}
