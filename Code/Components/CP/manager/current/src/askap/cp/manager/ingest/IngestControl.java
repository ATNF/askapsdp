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
 */
package askap.cp.manager.ingest;

// System imports
import java.util.Map;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import askap.interfaces.cp.AlreadyRunningException;

// Local package includes
import askap.cp.manager.rman.IJob;
import askap.cp.manager.rman.IJob.JobStatus;
import askap.cp.manager.rman.IResourceManager;

public class IngestControl {
	
	/** Logger. */
	private static Logger logger = Logger.getLogger(IngestControl.class.getName());
	
	IResourceManager itsResourceManager;
	
	/**
	 * An identifier for the ingest pipeline job in the batch scheduler. This
	 * Will be an empty string when no ingest pipeline is running.
	 */
	IJob itsJob = null;
    
    public IngestControl(IResourceManager rman) {
    	// Pre-conditions
    	assert(rman != null);
    	
    	logger.info("Creating IngestControl");
    	itsResourceManager = rman;
    }
	
	public void start(Map<String, String> facilityConfig, Map<String, String> obsParams)
		throws askap.interfaces.cp.AlreadyRunningException {
		logger.info("Starting Ingest Pipeline");
		
		// 1: Check the pipeline is not already running
		if ((itsJob != null) && (itsJob.status() != JobStatus.COMPLETED)) {
				String msg = "Ingest Pipeline already running. JobId: " + itsJob.toString();
				throw new AlreadyRunningException(msg);
		}
		
		// 2: Build the configuration for cpingest
		
		// 3: Execute the job
		
		// 4: Wait until the job is running or completed
		while (itsJob.status() == JobStatus.QUEUED) {
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) { }
		}
		
		// Post-conditions
		assert(itsJob != null);
	}
	
	public void abort() {
		logger.info("Aborting Ingest Pipeline");
		itsJob.abort();
		while (itsJob.status() != JobStatus.COMPLETED) {
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) { }
		}
		itsJob = null;
	}
	
}
