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
package askap.cp.sms;

// ASKAPsoft imports
import Ice.Current;
import askap.interfaces.skymodelservice.Component;
import askap.interfaces.skymodelservice._ISkyModelServiceDisp;
import org.apache.log4j.Logger;


/**
 * 
 */
public class SkyModelServiceImpl extends _ISkyModelServiceDisp {

	/** Logger. */
	private static Logger logger = Logger.getLogger(SkyModelServiceImpl.class.getName());
	
	/** Ice Communicator */
	private Ice.Communicator itsComm;
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public SkyModelServiceImpl(Ice.Communicator ic) {
		logger.info("Creating SkyModelService");
		itsComm = ic;
	}
	
	public void finalize() {
		logger.info("Destroying SkyModelService");
	}
	
	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#coneSearch(double, double, double, Ice.Current)
	 */
	public long[] coneSearch(double arg0, double arg1, double arg2, Current arg3) {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#getComponents(long[], Ice.Current)
	 */
	public Component[] getComponents(long[] arg0, Current arg1) {
		// TODO Auto-generated method stub
		return null;
	}

}
