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
package askap.cp.manager.svcclients;

// ASKAPsoft imports
import askap.interfaces.schedblock.NoSuchSchedulingBlockException;
import askap.util.ParameterSet;

/**
 * This class provides a mock implementation of IDataServiceClient. It is
 * instantiated with the observation parameters for sbid == 0 and will return
 * the obsparameters when getObsParameters() is called.
 * 
 */
public class MockDataServiceClient implements IDataServiceClient {

	/**
	 * The obs parameters for the mock schedulgin block with id of 0
	 */
	private ParameterSet itsObsParams;
	
	/**
	 * Constructor.
	 * 
	 * @param obsparams
	 */
	public MockDataServiceClient(ParameterSet obsparams) {
		itsObsParams = obsparams;
	}
	
	/*
	 * @see askap.cp.manager.svcclients.IDataServiceClient#getObsParameters(long)
	 */
	@Override
	public ParameterSet getObsParameters(long sbid)
			throws NoSuchSchedulingBlockException {
		if (sbid != 0) {
			throw new NoSuchSchedulingBlockException();
		}
		return itsObsParams;
	}

}
