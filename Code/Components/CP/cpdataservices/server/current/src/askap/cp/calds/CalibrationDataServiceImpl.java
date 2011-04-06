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

// Java imports
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

//ASKAPsoft imports
import org.apache.log4j.Logger;
import Ice.Current;
import askap.cp.calds.persist.GainSolutionElementBean;
import askap.cp.calds.persist.PersistenceInterface;
import askap.cp.calds.persist.TimeTaggedGainSolutionBean;
import askap.interfaces.caldataservice._ICalibrationDataServiceDisp;
import askap.interfaces.calparams.CalibrationParameters;
import askap.interfaces.calparams.JonesIndex;
import askap.interfaces.calparams.JonesJTerm;
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
	public void addGainsSolution(TimeTaggedGainSolution solution, Current cur) {
		itsPersistance.addGainSolution(solution);

	}

	/**
	 * @see askap.interfaces.caldataservice._ICalDataServiceOperations#addBandpassSolution(askap.interfaces.calparams.TimeTaggedBandpassSolution, Ice.Current)
	 */
	@Override
	public void addBandpassSolution(TimeTaggedBandpassSolution solution, Current cur) {
		//itsPersistance.addBandpassSolution(iceToBean(solution));
	}

	/**
	 * @see askap.interfaces.caldataservice._ICalDataServiceOperations#addLeakageSolution(askap.interfaces.calparams.TimeTaggedLeakageSolution, Ice.Current)
	 */
	@Override
	public void addLeakageSolution(TimeTaggedLeakageSolution solution, Current cur) {
		//itsPersistance.addLeakageSolution(iceToBean(solution));

	}

	/**
	 * @see askap.interfaces.caldataservice._ICalibrationDataServiceOperations#getCurrentSolution(Ice.Current)
	 */
	@Override
	public Map<JonesIndex, CalibrationParameters> getCurrentSolution(Current cur) {
		// TODO Auto-generated method stub
		return null;
	}
	


}
