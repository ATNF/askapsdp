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

// Java imports
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

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
	private static Logger logger = Logger.getLogger(SkyModelServiceImpl.class
			.getName());

	/** Ice Communicator */
	@SuppressWarnings("unused")
	private Ice.Communicator itsComm;

	private PersistenceInterface itsPersistance;
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public SkyModelServiceImpl(Ice.Communicator ic) {
		logger.info("Creating SkyModelService");
		itsComm = ic;
		itsPersistance = new PersistenceInterface();
	}

	public void finalize() {
		logger.info("Destroying SkyModelService");
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#coneSearch(double, double, double, Ice.Current)
	 */
	@Override
	public List<Long> coneSearch(double ra, double dec, double searchRadius,
			Current cur) {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#getComponents(java.util.List, Ice.Current)
	 */
	@Override
	public List<Component> getComponents(List<Long> componentIds, Current cur) {
		List<ComponentBean> beans = itsPersistance.getComponents(componentIds);
		List<Component> components = convertBeanToIce(beans);
		return components;
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#addComponents(java.util.List, Ice.Current)
	 */
	@Override
	public List<Long> addComponents(List<Component> components, Current cur) {
		List<Long> ids = itsPersistance.addComponents(convertIceToBean(components));
		return ids;
	}

	/**
	 * @see askap.interfaces.skymodelservice._ISkyModelServiceOperations#removeAllComponents(Ice.Current)
	 */
	@Override
	public void removeAllComponents(Current cur) {
		itsPersistance.removeAllComponents();
	}
	
	List<askap.interfaces.skymodelservice.Component> convertBeanToIce(List<ComponentBean> beans) {
		ArrayList<Component> components = new ArrayList<Component>();
		
		Iterator<ComponentBean> it = beans.iterator();
		while (it.hasNext()) {
			ComponentBean cb = it.next();
			Component c = new Component();
			c.id = cb.getId();
			c.rightAscension = cb.getRightAscension();
			c.declination = cb.getDeclination();
			c.positionAngle = cb.getPositionAngle();
			c.majorAxis = cb.getMajorAxis();
			c.minorAxis = cb.getMinorAxis();
			c.i1400 = cb.getI1400();
			components.add(c);
		}
		return components;
	}
	
	List<ComponentBean> convertIceToBean(List<askap.interfaces.skymodelservice.Component> components) {
		ArrayList<ComponentBean> beans = new ArrayList<ComponentBean>();
		
		Iterator<Component> it = components.iterator();
		while (it.hasNext()) {
			Component c = it.next();
			ComponentBean cb = new ComponentBean();
			cb.setRightAscension(c.rightAscension);
			cb.setDeclination(c.declination);
			cb.setPositionAngle(c.positionAngle);
			cb.setMajorAxis(c.majorAxis);
			cb.setMinorAxis(c.minorAxis);
			cb.setI1400(c.i1400);
			beans.add(cb);
		}
		return beans;
	}
}
