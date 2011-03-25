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
import org.apache.log4j.Logger;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.cfg.Configuration;

/**
 * 
 */
public class PersistenceInterface {
	/** Logger. */
	private static Logger logger = Logger.getLogger(PersistenceInterface.class
			.getName());
	
	/**
	 * Hibernate session
	 */
	private Session itsSession = null;
	
	/**
	 * Constructor
	 */
	public PersistenceInterface() {
		logger.debug("Creating " + PersistenceInterface.class.getName() );
		
		// A SessionFactory is set up for the persistence interface
		Configuration config = new Configuration();
		config.configure("skymodel-hibernate.cfg.xml");
		SessionFactory sessionFactory = config.buildSessionFactory();
		itsSession = sessionFactory.openSession();
	}
	
	/**
	 * Constructor.
	 * Used for testing.
	 * 
	 * @param session
	 */
	public PersistenceInterface(org.hibernate.Session session) {
		itsSession = session;
	}

	protected void finalize() {
		itsSession.close();
	}
	
	/**
	 * @param ra
	 * @param dec
	 * @param searchRadius
	 * @return
	 */
	public List<Long> coneSearch(double ra, double dec, double searchRadius) {
		ArrayList<Long> ids = new ArrayList<Long>();
		
		// TODO: This code needs to do a database selection based on RA and DEC so
		// as not to have the entire table returned
		
		itsSession.beginTransaction();
		@SuppressWarnings("unchecked")
		List<ComponentBean> result = (List<ComponentBean>) itsSession.createQuery( "from ComponentBean" ).list();
		itsSession.getTransaction().commit();
		for ( ComponentBean comp : (List<ComponentBean>) result ) {
			// TODO: The below uses simple (Euclidean geometry) Pythagoras
			// theorem. Need to handle spherical geometry, wrap-around etc.
			final double a = Math.abs(dec) - Math.abs(comp.getDeclination());
			final double b = Math.abs(ra) - Math.abs(comp.getRightAscension());
			final double c = Math.sqrt((Math.pow(a, 2) + Math.pow(b, 2)));
			if (c <= searchRadius) {
				ids.add(new Long(comp.getId()));
			}
		}
		
		return ids;
	}


	/**
	 * @param componentIds
	 * @return
	 */
	public List<ComponentBean> getComponents(List<Long> componentIds) {
		ArrayList<ComponentBean> components = new ArrayList<ComponentBean>();
		
		Iterator<Long> it = componentIds.iterator();
		while(it.hasNext()) {
			ComponentBean c = (ComponentBean) itsSession.load(ComponentBean.class, it.next());
			components.add(c);
		}
				
		return components;
	}


	/**
	 * @param components
	 * @return
	 */
	public List<Long> addComponents(List<ComponentBean> components) {
		ArrayList<Long> idList = new ArrayList<Long>();
		
		Iterator<ComponentBean> it = components.iterator();
		itsSession.beginTransaction();
	    while(it.hasNext()) {
	    	ComponentBean c = it.next();
	    	itsSession.save(c);
	    	idList.add(new Long(c.getId()));
	    	
	    }
	    itsSession.getTransaction().commit();
		
		return idList;
	}


	/**
	 * 
	 */
	public void removeAllComponents() {
		logger.warn("removeAllComponents() is not yet implemented");
	}
}
