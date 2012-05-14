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
import askap.interfaces.schedblock.ISchedulingBlockServicePrx;
import askap.interfaces.schedblock.ISchedulingBlockServicePrxHelper;
import askap.interfaces.schedblock.NoSuchSchedulingBlockException;
import askap.util.ParameterSet;

public class IceDataServiceClient implements IDataServiceClient {
	
	/**
	 * Logger.
	 */
	private static Logger logger = Logger.getLogger(IceDataServiceClient.class.getName());
	
	ISchedulingBlockServicePrx itsProxy;
	
	public IceDataServiceClient(Ice.Communicator ic) {
		logger.info("Creating DataServiceClient");
		Ice.ObjectPrx obj = ic.stringToProxy("SchedulingBlockService");
		itsProxy = ISchedulingBlockServicePrxHelper.checkedCast(obj);
	}
	
	/**
	 * @see askap.cp.manager.svcclients.IDataServiceClient#getObsParameters(long)
	 */
	public ParameterSet getObsParameters(long sbid)
			throws NoSuchSchedulingBlockException {
		return new ParameterSet(itsProxy.getObsParameters(sbid));
	}
}
