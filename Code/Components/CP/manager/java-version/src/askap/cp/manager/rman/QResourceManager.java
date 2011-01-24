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
package askap.cp.manager.rman;

// System imports
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Map;

// ASKAPsoft imports
import org.apache.log4j.Logger;

import askap.cp.manager.rman.JobTemplate.DependType;

/**
 * An implementation of IResourceManager which uses the qsub/qstat command
 * line interface.
 * @author Ben Humphreys <ben.humphreys@csiro.au>
 */
public class QResourceManager implements IResourceManager {

	/** Logger. */
	private static Logger logger = Logger.getLogger(
			QResourceManager.class.getName());

	/**
	 * @see askap.cp.manager.rman.IResourceManager#getStatus()
	 */
	public ServerStatus getStatus() {
		int status = -1;
		try {
			Process p = Runtime.getRuntime().exec("qsub");
			status = p.waitFor();
		} catch (IOException e) {
			logger.info("Failed to exec: " + e.getMessage());
			e.printStackTrace();
		} catch (InterruptedException e) {
			logger.info("InterruptedException: " + e.getMessage());
			e.printStackTrace();
		}

		if (status == 0) {
			return ServerStatus.AVAILABLE;
		} else {
			return ServerStatus.UNCONTACTABLE;
		}
	}

	/**
	 * @see askap.cp.manager.rman.IResourceManager#submitJob(askap.cp.manager.rman.JobTemplate,
	 *      java.lang.String)
	 */
	public IJob submitJob(JobTemplate template, String queue) {
		// Build the command
		String cmd = "qsub " + buildDependencyArg(template)
			+ template.getScriptLocation();

		// Submit the job
		int status = -1;
		try {
			Process p = Runtime.getRuntime().exec(cmd);
			status = p.waitFor();

			if (status == 0) {
				BufferedReader stdout = new BufferedReader(
						new InputStreamReader(
								p.getInputStream()));
				String jobid = stdout.readLine();
				logger.debug("Submitted job with id: " + jobid);

				return new QJob(jobid);
			} else {
				throw new RuntimeException("Error " + status + " submitting job");
			}
		} catch (Exception e) {
			logger.error(e.getClass().getName() + ": " + e.getMessage());
			e.printStackTrace();
			throw new RuntimeException(e.getClass().getName());
		}
	}

	/**
	 * Builds the dependency argument (to qsub) based on the dependencies
	 * listed in the job template.
	 * @param template	the job template which contains the dependencies.
	 * @return the dependency argument to be passed to qsub
	 */
	String buildDependencyArg(JobTemplate template) {
		Map<IJob, DependType> dependencies = template.getDependencies();
		if (dependencies.size() == 0) {
			return "";
		}

		String str = "-W depend=";
		int count = 0;
		for (Map.Entry<IJob, DependType> entry : dependencies.entrySet()) {
			if (count > 0) {
				str = str + ",";
			}
			QJob job = (QJob)entry.getKey();
			switch (entry.getValue()) {
			case AFTERSTART:
				str = str + "after:" + job.getId();
				break;
			case AFTEROK:
				str = str + "afterok:" + job.getId();
				break;
			case AFTERNOTOK:
				str = str + "afternotok:" + job.getId();
				break;
			default:
				logger.error("Unhandled dependency type");
				break;
			}
			count++;
		}
		return str;
	}

}
