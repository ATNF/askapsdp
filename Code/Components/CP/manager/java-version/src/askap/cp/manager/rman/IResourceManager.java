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

/**
 * Interface to the resource manager.
 * 
 * @author Ben Humphreys <ben.humphreys@csiro.au>
 */
public interface IResourceManager {
	/**
	 * The state of the batch queue server
	 */
	public enum ServerStatus {AVAILABLE, UNCONTACTABLE}

	/**
	 * @return an enum containing the state of the server.
	 */
	public ServerStatus getStatus();
	
    /**
     * Submit a new job for execution
     * @param jobTemplate	template for the job to submit.
     * @param queue		name of the queue to submit the job to.
     * @return			a object which references the submitted job.
     */
    public IJob submitJob(JobTemplate jobTemplate, final String queue);
}
