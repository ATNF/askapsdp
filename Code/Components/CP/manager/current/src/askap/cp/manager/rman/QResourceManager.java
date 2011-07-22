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
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

// ASKAPsoft imports
import org.apache.log4j.Logger;

/**
 * An implementation of IResourceManager which uses the qsub/qstat command
 * line interface.
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
			Process p = Runtime.getRuntime().exec("qstat");
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
	public IJob submitJob(final File jobfile, final File workdir) {
		String cmd = "qsub " + jobfile.getPath();
		// Submit the job
		int status = -1;
		try {
			Process p = Runtime.getRuntime().exec(cmd, null, workdir);
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

}
