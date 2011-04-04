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
import askap.cp.sms.SkyModelServiceImpl;
import askap.interfaces.caldataservice._ICalibrationDataServiceDisp;
import askap.interfaces.calparams.TimeTaggedBandpassSolution;
import askap.interfaces.calparams.TimeTaggedGainSolution;
import askap.interfaces.calparams.TimeTaggedLeakageSolution;

/**
 * @author hum092
 *
 */
public class CalibrationDataServiceImpl extends _ICalibrationDataServiceDisp {
	
	/** 
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(SkyModelServiceImpl.class
			.getName());
	
	private static final long serialVersionUID = 1L;
	
	/**
	 * Constructor
	 */
	public CalibrationDataServiceImpl(Ice.Communicator ic) {
		logger.info("Creating Calibration Data Service");
	}

	/**
	 * finalize
	 */
	public void finalize() {
		logger.info("Destroying Calibration Data Service");
	}
	

	/* (non-Javadoc)
	 * @see askap.interfaces.caldataservice._ICalDataServiceOperations#addBandpassSolution(askap.interfaces.calparams.TimeTaggedBandpassSolution, Ice.Current)
	 */
	@Override
	public void addBandpassSolution(TimeTaggedBandpassSolution solution,
			Current cur) {
		// TODO Auto-generated method stub

	}

	/* (non-Javadoc)
	 * @see askap.interfaces.caldataservice._ICalDataServiceOperations#addGainsSolution(askap.interfaces.calparams.TimeTaggedGainSolution, Ice.Current)
	 */
	@Override
	public void addGainsSolution(TimeTaggedGainSolution solution, Current cur) {
		// TODO Auto-generated method stub

	}

	/* (non-Javadoc)
	 * @see askap.interfaces.caldataservice._ICalDataServiceOperations#addLeakageSolution(askap.interfaces.calparams.TimeTaggedLeakageSolution, Ice.Current)
	 */
	@Override
	public void addLeakageSolution(TimeTaggedLeakageSolution solution, Current cur) {
		// TODO Auto-generated method stub

	}

}
