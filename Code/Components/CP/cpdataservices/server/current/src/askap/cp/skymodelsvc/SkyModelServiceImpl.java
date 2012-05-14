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
package askap.cp.skymodelsvc;

// Java imports
import java.util.List;

// ASKAPsoft imports
import Ice.Current;
import askap.cp.skymodelsvc.persist.JDBCPersistenceInterface;
import askap.interfaces.skymodelservice.Component;
import askap.interfaces.skymodelservice._ISkyModelServiceDisp;
import org.apache.log4j.Logger;

/**
 * 
 */
public class SkyModelServiceImpl extends _ISkyModelServiceDisp {

	/** 
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(SkyModelServiceImpl.class
			.getName());

	/**
	 * Class which provides access to the persistence layer
	 */
	private JDBCPersistenceInterface itsPersistance;
	
	private static final long serialVersionUID = 1L;

	/**
	 * Constructor
	 */
	public SkyModelServiceImpl(Ice.Communicator ic) {
		logger.info("Creating SkyModelService");
		itsPersistance = new JDBCPersistenceInterface();
	}

	/**
	 * finalize
	 */
	public void finalize() {
		logger.info("Destroying SkyModelService");
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#coneSearch(double, double, double, Ice.Current)
	 */
	@Override
	public List<Long> coneSearch(double ra, double dec, double searchRadius,
			double fluxLimit, Current cur) {
		return itsPersistance.coneSearch(ra, dec, searchRadius, fluxLimit);
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#getComponents(java.util.List, Ice.Current)
	 */
	@Override
	public List<Component> getComponents(List<Long> componentIds, Current cur) {
		return itsPersistance.getComponents(componentIds);
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#addComponents(java.util.List, Ice.Current)
	 */
	@Override
	public List<Long> addComponents(List<Component> components, Current cur) {
		return itsPersistance.addComponents(components);
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#removeComponents(java.util.List, Ice.Current)
	 */
	@Override
	public void removeComponents(List<Long> componentIds, Current cur) {
		itsPersistance.removeComponents(componentIds);
	}
}
