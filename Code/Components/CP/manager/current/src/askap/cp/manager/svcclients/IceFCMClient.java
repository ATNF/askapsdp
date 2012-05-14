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
package askap.cp.manager.svcclients;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import askap.interfaces.fcm.IFCMServicePrx;
import askap.interfaces.fcm.IFCMServicePrxHelper;
import askap.interfaces.fcm.NoSuchKeyException;
import askap.util.ParameterSet;

public class IceFCMClient implements IFCMClient {
	
	/**
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(IceFCMClient.class.getName());
	
	IFCMServicePrx itsProxy;
	
	public IceFCMClient(Ice.Communicator ic) {
		logger.info("Creating FCMClient");
		Ice.ObjectPrx obj = ic.stringToProxy("FCMService");
		itsProxy = IFCMServicePrxHelper.checkedCast(obj);
	}
	
	/**
	 * @see askap.cp.manager.svcclients.IFCMClient#get()
	 */
	public ParameterSet get() {
		try {
			return new ParameterSet(itsProxy.get(-1, ""));
		} catch (NoSuchKeyException e) {
			// Shouldn't get this because we are not specifing a key.
			return new ParameterSet();
		}
	}
}
