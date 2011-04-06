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
package askap.cp.calds.persist;

import java.util.ArrayList;
import java.util.List;

public class TimeTaggedGainSolutionBean {
	long itsId;
	
	long itsTime = 0;
	
	List<GainSolutionElementBean> itsElements = new ArrayList<GainSolutionElementBean>();

	/**
	 * Constructor.
	 * 
	 * @param time
	 * @param solutionElements
	 */
	public TimeTaggedGainSolutionBean(long time, List<GainSolutionElementBean> solutionElements)
	{
		this.itsTime = time;
		this.itsElements = solutionElements;
	}
	
	/**
	 * @return the id
	 */
	public Long getId() {
		return itsId;
	}

	/**
	 * @param id the id to set
	 */
	public void setId(Long id) {
		this.itsId = id;
	}

	/**
	 * @return the time
	 */
	public long getTime() {
		return itsTime;
	}

	/**
	 * @param time the time to set
	 */
	public void setTime(long time) {
		this.itsTime = time;
	}

	/**
	 * @return the solution
	 */
	public List<GainSolutionElementBean> getSolutionElements() {
		return itsElements;
	}

	/**
	 * @param solution the solution to set
	 */
	public void setSolutionElements(List<GainSolutionElementBean> solutionElements) {
		this.itsElements = solutionElements;
	}
}
