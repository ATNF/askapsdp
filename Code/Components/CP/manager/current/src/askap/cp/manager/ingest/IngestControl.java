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

// Core Java imports
import java.io.File;
import java.io.IOException;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import askap.interfaces.cp.AlreadyRunningException;
import askap.util.ParameterSet;

// Local package includes
import askap.cp.manager.rman.IJob;
import askap.cp.manager.rman.IJob.JobStatus;
import askap.cp.manager.rman.IResourceManager;

public class IngestControl {

	/**
	 * Logger
	 * */
	private static Logger logger = Logger.getLogger(IngestControl.class
			.getName());

	/**
	 * The resource managaer; provides an interface to PBS, Torque, etc.
	 */
	private IResourceManager itsResourceManager;

	/**
	 * Parameter set (i.e. configuration from file)
	 */
	private ParameterSet itsParset;

	/**
	 * An identifier for the ingest pipeline job in the batch scheduler. This
	 * Will be an empty string when no ingest pipeline is running.
	 */
	IJob itsJob = null;

	/**
	 * Constructor
	 * 
	 * @param rman
	 *            an instance of a resource manager.
	 */
	public IngestControl(IResourceManager rman, ParameterSet parset) {
		// Pre-conditions
		assert (rman != null);

		logger.info("Creating IngestControl");
		itsResourceManager = rman;
		itsParset = parset;
	}

	/**
	 * Calling this method instructs the central processor to carry out the
	 * observation described in the parameter set associated with the scheduling
	 * block.
	 * 
	 * This method, when called will block until such times as the central
	 * processor is ready to begin receiving data from the correlator and
	 * telescope operating system. The central processor takes some time to
	 * prepare for an observation (on the order of a few seconds), hence the
	 * need to indicate when it is ready by blocking.
	 * 
	 * @param facilityConfig
	 * @param obsParams
	 * @throws askap.interfaces.cp.AlreadyRunningException
	 */
	public void startIngest(ParameterSet facilityConfig,
			ParameterSet obsParams, long sbid)
			throws askap.interfaces.cp.AlreadyRunningException {
		// Pre-conditions
		assert (itsResourceManager != null);

		logger.info("Starting Ingest Pipeline");

		// 1: Check the pipeline is not already running
		if ((itsJob != null) && (itsJob.status() != JobStatus.COMPLETED)) {
			String msg = "Ingest Pipeline already running. JobId: "
					+ itsJob.toString();
			throw new AlreadyRunningException(msg);
		} else {
			// This means the last running ingest job is completed and hence
			// the reference to it can be discarded
			itsJob = null;
		}

		// 2: Build the configuration for cpingest
		ParameterSet parset = ConfigBuilder.build(facilityConfig, obsParams);

		// 3: Create the directory for the scheduling block and create
		// the config parset and qsub file
		File workdir = new File(itsParset.getString("ingest.workdir") + "/" + sbid);
		File jobtemplate = new File(itsParset.getString("ingest.job_template"));
		File qsubFile = new File(workdir, "ingest.qsub");
		File configFile = new File(workdir, "ingest.in");

		FSUtils.mkdir(workdir);
		try {
			FSUtils.copyfile(jobtemplate, qsubFile);
		} catch (IOException e) {
			logger.error("Could not copy job template to workdir");
			return;
		}
		try {
			FSUtils.create(configFile, parset);
		} catch (IOException e) {
			logger.error("Could not create parset in wordir");
			return;
		}

		// 4: Execute the job
		itsJob = itsResourceManager.submitJob(qsubFile, workdir);

		// 5: Wait until the job is running or completed
		while (itsJob.status() == JobStatus.QUEUED) {
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
			}
		}

		// Post-conditions
		assert (itsJob != null);
	}

	/**
	 * Blocks until the observation in progress is completed. Specifically,
	 * until the ingest pipeline finishes, either successfully or with error.
	 * When this method returns, the central processor is ready to start a new
	 * observation.
	 */
	public void waitIngest() {
		while (itsJob != null && itsJob.status() != JobStatus.COMPLETED) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
			}
		}
		itsJob = null;
	}

	/**
	 * Calling this method instructs the central processor to abort the current
	 * observation. This stops the data acquisition process.
	 * 
	 * This method will block until the observation has been aborted and the
	 * central processor is ready to start a new observation.
	 */
	public void abortIngest() {
		logger.info("Aborting Ingest Pipeline");
		if (itsJob == null) {
			return;
		}
		itsJob.abort();
		while (itsJob.status() != JobStatus.COMPLETED) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
			}
		}
		itsJob = null;
	}

}
