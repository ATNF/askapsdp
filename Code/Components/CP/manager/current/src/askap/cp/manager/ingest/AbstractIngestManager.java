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

import java.io.File;
import java.io.IOException;

import org.apache.log4j.Logger;
import askap.interfaces.cp.AlreadyRunningException;
import askap.util.ParameterSet;


/**
 * Encapsulates control and management of the central processor ingest pipeline.
 */
public abstract class AbstractIngestManager {

	/** Logger */
	private static Logger logger = Logger.getLogger(AbstractIngestManager.class
			.getName());

	/**
	 * Parameter set (i.e. configuration from file)
	 */
	private ParameterSet itsParset;

	/**
	 * Constructor
	 */
	public AbstractIngestManager(ParameterSet parset) {
		// Pre-conditions
		assert (parset != null);
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
	public synchronized void startIngest(ParameterSet facilityConfig,
			ParameterSet obsParams, long sbid)
			throws askap.interfaces.cp.AlreadyRunningException {

		logger.info("Starting Ingest Pipeline");

		// 1: Check the pipeline is not already running
		if (isRunning()) {
			String msg = "Ingest Pipeline already running";
			throw new AlreadyRunningException(msg);
		}

		// 2: Build the configuration for cpingest
		ParameterSet parset = ConfigBuilder.build(facilityConfig, obsParams);

		// 3: Create the directory for the scheduling block and create
		// the config parset file
		File workdir = new File(parset().getString("ingest.workdir") + "/" + sbid);
		File configFile = new File(workdir, "cpingest.in");

		FSUtils.mkdir(workdir);
		try {
			FSUtils.create(configFile, parset);
		} catch (IOException e) {
			logger.error("Could not create parset in workdir: " + e);
			return;
		}

		// 4: Execute the job (blocks until pipeline is running)
		executeIngestPipeline(workdir);
	}

	/**
	 * Calling this method instructs the central processor to abort the current
	 * observation. This stops the data acquisition process.
	 * 
	 * This method will block until the observation has been aborted and the
	 * central processor is ready to start a new observation.
	 */
	public synchronized void abortIngest() {
		logger.info("Aborting Ingest Pipeline");

		// Blocks until the pipeline has been aborts
		abortIngestPipeline();
	}
	
	/**
	 * Concrete classes implement this to start/execute the ingest pipeline.
	 */
	abstract protected void executeIngestPipeline(File workdir);
			
	/**
	 * Concrete classes implement this to abort/kill the ingest pipeline.
	 */
	abstract protected void abortIngestPipeline();
	
	/**
	 * Concrete classes implement this to indicate if the pipeline is
	 * already running.
	 * @return true if the pipeline is running, otherwise false.
	 */
	abstract protected boolean isRunning();
	
	/**
	 * @return the parset passed to this class when it was constructed
	 */
	protected ParameterSet parset() {
		return itsParset;
	}

}
