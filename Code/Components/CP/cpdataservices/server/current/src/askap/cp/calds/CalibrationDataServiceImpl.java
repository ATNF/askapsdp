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

//ASKAPsoft imports
import org.apache.log4j.Logger;
import Ice.Current;
import askap.cp.calds.persist.PersistenceInterface;
import askap.interfaces.caldataservice._ICalibrationDataServiceDisp;
import askap.interfaces.calparams.TimeTaggedBandpassSolution;
import askap.interfaces.calparams.TimeTaggedGainSolution;
import askap.interfaces.calparams.TimeTaggedLeakageSolution;

/**
 * Implementation of the Calibration Data Service ICE interface.
 * @author Ben Humphreys
 */
public class CalibrationDataServiceImpl extends _ICalibrationDataServiceDisp {
	
	/** 
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(CalibrationDataServiceImpl.class
			.getName());
	
	private static final long serialVersionUID = 1L;
	
	private long itsOptimumGainID;
	private long itsOptimumLeakageID;
	private long itsOptimumBandpassID;
	
	/**
	 * Class which provides access to the persistence layer
	 */
	private PersistenceInterface itsPersistance;
	
	/**
	 * Constructor
	 */
	public CalibrationDataServiceImpl(Ice.Communicator ic) {
		logger.info("Creating Calibration Data Service");
		itsPersistance = new PersistenceInterface();
		
		// TODO: Currently, the optimum ID is not known unless a call to add*()
		// occurred. Need to read the database to find the best one on startup.
		itsOptimumGainID = -1;
		itsOptimumLeakageID = -1;
		itsOptimumBandpassID = -1;
	}

	/**
	 * finalize
	 */
	public void finalize() {
		logger.info("Destroying Calibration Data Service");
	}
	
	/**
	 * @see askap.interfaces.caldataservice._ICalDataServiceOperations#addGainsSolution(askap.interfaces.calparams.TimeTaggedGainSolution, Ice.Current)
	 */
	@Override
	public long addGainsSolution(TimeTaggedGainSolution solution, Current cur) {
		if (solution.gain == null) {
			logger.warn("Hash in passed gain solution is null");
			return -1;
		}
		
		final long id = itsPersistance.addGainSolution(solution);
		itsOptimumGainID = id;
		return id;
	}

	/**
	 * @see askap.interfaces.caldataservice._ICalDataServiceOperations#addLeakageSolution(askap.interfaces.calparams.TimeTaggedLeakageSolution, Ice.Current)
	 */
	@Override
	public long addLeakageSolution(TimeTaggedLeakageSolution solution, Current cur) {
		if (solution.leakage == null) {
			logger.warn("Hash in passed leakage solution is null");
			return -1;
		}
		
		final long id = itsPersistance.addLeakageSolution(solution);
		itsOptimumLeakageID = id;
		return id;
	}
	
	/**
	 * @see askap.interfaces.caldataservice._ICalDataServiceOperations#addBandpassSolution(askap.interfaces.calparams.TimeTaggedBandpassSolution, Ice.Current)
	 */
	@Override
	public long addBandpassSolution(TimeTaggedBandpassSolution solution, Current cur) {
		if (solution.bandpass == null) {
			logger.warn("Hash in passed bandpass solution is null");
			return -1;
		}
		
		final long id = itsPersistance.addBandpassSolution(solution); 
		itsOptimumBandpassID = id;
		return id;
	}

	/**
	 * @see askap.interfaces.caldataservice._ICalibrationDataServiceOperations#getCurrentGainSolutionID(Ice.Current)
	 */
	@Override
	public long getCurrentGainSolutionID(Current cur) {
		return itsOptimumGainID;
	}

	/**
	 * @see askap.interfaces.caldataservice._ICalibrationDataServiceOperations#getCurrentLeakageSolutionID(Ice.Current)
	 */
	@Override
	public long getCurrentLeakageSolutionID(Current cur) {
		return itsOptimumLeakageID;
	}
	
	/**
	 * @see askap.interfaces.caldataservice._ICalibrationDataServiceOperations#getCurrentBandpassSolutionID(Ice.Current)
	 */
	@Override
	public long getCurrentBandpassSolutionID(Current cur) {
		return itsOptimumBandpassID;
	}

	/**
	 * @see askap.interfaces.caldataservice._ICalibrationDataServiceOperations#getGainSolution(long, Ice.Current)
	 */
	@Override
	public TimeTaggedGainSolution getGainSolution(long id, Current cur) {
		return itsPersistance.getGainSolution(id);
	}

	/**
	 * @see askap.interfaces.caldataservice._ICalibrationDataServiceOperations#getLeakageSolution(long, Ice.Current)
	 */
	@Override
	public TimeTaggedLeakageSolution getLeakageSolution(long id, Current cur) {
		return itsPersistance.getLeakageSolution(id);
	}

	/**
	 * @see askap.interfaces.caldataservice._ICalibrationDataServiceOperations#getBandpassSolution(long, Ice.Current)
	 */
	@Override
	public TimeTaggedBandpassSolution getBandpassSolution(long id, Current cur) {
		return itsPersistance.getBandpassSolution(id);
	}
}
