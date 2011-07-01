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
package askap.cp.caldatasvc.persist;

public class BandpassSolutionBean {
	
	private long itsID;
	private long itsTimestamp;
	private int itsNChan;
	
	/**
	 * Constructor
	 * @note This no-args constructor is needed by Hibernate
	 */
	public BandpassSolutionBean() {
		itsID = -1;
		itsTimestamp = -1;
		itsNChan = -1;
	}
	
	public BandpassSolutionBean(long timestamp, int nChan) {
		itsTimestamp = timestamp;
		itsNChan = nChan;
	}
	
	/**
	 * @return the ID
	 */
	public long getID() {
		return itsID;
	}

	/**
	 * @param id the ID to set
	 */
	public void setID(long id) {
		itsID = id;
	}

	/**
	 * @return the timestamp
	 */
	public long getTimestamp() {
		return itsTimestamp;
	}

	/**
	 * @param timestamp the timestamp to set
	 */
	public void setTimestamp(long timestamp) {
		itsTimestamp = timestamp;
	}

	/**
	 * @note Using lowercase "n" in getnChan() to make Hibernate happy
	 * @return the nChan
	 */
	public int getnChan() {
		return itsNChan;
	}

	/**
	 * @note Using lowercase "n" in getnChan() to make Hibernate happy
	 * @param nChan the nChan to set
	 */
	public void setnChan(int nChan) {
		itsNChan = nChan;
	}

}
