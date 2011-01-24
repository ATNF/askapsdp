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
 * @author Ben Humphreys <ben.humphreys@csiro.au>
 */
public class QJob implements IJob {

	private String itsId = null;
	
	QJob(String id) {
		itsId = id;
	}
	
	/**
	 * @see askap.cp.manager.rman.IJob#status()
	 */
	public JobStatus status() {
		return JobStatus.UNKNOWN;
	}

	/**
	 * @see askap.cp.manager.rman.IJob#abort()
	 */
	public void abort() {
	}
	
	public String getId()
	{
		return itsId;
	}
	
	/**
	 * @see java.lang.Object#hashCode()
	 */
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((itsId == null) ? 0 : itsId.hashCode());
		return result;
	}

	/**
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		QJob other = (QJob) obj;
		if (itsId == null) {
			if (other.itsId != null)
				return false;
		} else if (!itsId.equals(other.itsId))
			return false;
		return true;
	}
}
