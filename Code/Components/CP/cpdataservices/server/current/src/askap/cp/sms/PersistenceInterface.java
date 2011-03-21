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
	
	public List<Long> coneSearch(double ra, double dec, double searchRadius) {
		ArrayList<Long> ids = new ArrayList<Long>();
		
		itsSession.beginTransaction();
		@SuppressWarnings("unchecked")
		List<ComponentBean> result = (List<ComponentBean>) itsSession.createQuery( "from components" ).list();
		for ( ComponentBean c : (List<ComponentBean>) result ) {
		    System.out.println( "ComponentBean (" + c.getId() + ") : " + c.getI1400() );
		    ids.add(new Long(c.getId()));
		}
		itsSession.getTransaction().commit();
		
		return ids;
	}


	public List<ComponentBean> getComponents(List<Long> componentIds) {
		ArrayList<ComponentBean> components = new ArrayList<ComponentBean>();
		
		Iterator<Long> it = componentIds.iterator();
		while(it.hasNext()) {
			ComponentBean c = (ComponentBean) itsSession.load(ComponentBean.class, it.next());
			components.add(c);
		}
				
		return components;
	}


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


	public void removeAllComponents() {
		logger.warn("removeAllComponents() is not yet implemented");
	}
}
