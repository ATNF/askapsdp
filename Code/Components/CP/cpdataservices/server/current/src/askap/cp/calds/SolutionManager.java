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
package askap.cp.calds;

// ASKAPsoft imports
import org.apache.log4j.Logger;

// Local packages includes
import askap.cp.calds.persist.PersistenceInterface;
import askap.interfaces.calparams.TimeTaggedBandpassSolution;
import askap.interfaces.calparams.TimeTaggedGainSolution;
import askap.interfaces.calparams.TimeTaggedLeakageSolution;

public class SolutionManager {
	/**
	 * Logger
	 * */
	private static Logger logger = Logger.getLogger(SolutionManager.class
			.getName());
	
	/**
	 * Class which provides access to the persistence layer
	 */
	private PersistenceInterface itsPersistance;
	
	/**
	 * ID of the current best gain solution 
	 */
	private long itsOptimumGainID;
	
	/**
	 * ID of the current best leakage solution 
	 */
	private long itsOptimumLeakageID;
	
	/**
	 * ID of the current best bandpass solution 
	 */
	private long itsOptimumBandpassID;
	
	/**
	 * Constructor
	 */
	SolutionManager(org.hibernate.Session session) {
		itsPersistance = new PersistenceInterface(session);

		itsOptimumGainID = itsPersistance.getLatestGainSolution();
		itsOptimumLeakageID = itsPersistance.getLatestLeakageSolution();
		itsOptimumBandpassID = itsPersistance.getLatestBandpassSolution();
	}
	
	
	public long addGainSolution(TimeTaggedGainSolution solution) {
		final long id = itsPersistance.addGainSolution(solution);
		itsOptimumGainID = id;
		logger.info("New optimum gain solution is: " + id);
		return id;
	}
	
	public long addLeakageSolution(TimeTaggedLeakageSolution solution) {
		final long id = itsPersistance.addLeakageSolution(solution);
		itsOptimumLeakageID = id;
		logger.info("New optimum leakage solution is: " + id);
		return id;
	}

	public long addBandpassSolution(TimeTaggedBandpassSolution solution) {
		final long id = itsPersistance.addBandpassSolution(solution);
		itsOptimumBandpassID = id;
		logger.info("New optimum bandpass solution is: " + id);
		return id;
	}
	
	public long getCurrentGainSolutionID() {
		return itsOptimumGainID;
	}

	public long getCurrentLeakageSolutionID() {
		return itsOptimumLeakageID;
	}
	
	public long getCurrentBandpassSolutionID() {
		return itsOptimumBandpassID;
	}
	
	public TimeTaggedGainSolution getGainSolution(long id) {
		return itsPersistance.getGainSolution(id);
	}
	
	public TimeTaggedLeakageSolution getLeakageSolution(long id) {
		return itsPersistance.getLeakageSolution(id);
	}
	
	public TimeTaggedBandpassSolution getBandpassSolution(long id) {
		return itsPersistance.getBandpassSolution(id);
	}
}
