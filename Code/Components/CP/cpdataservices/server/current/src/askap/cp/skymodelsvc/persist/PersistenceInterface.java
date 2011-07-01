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
package askap.cp.skymodelsvc.persist;

// Java imports
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import org.hibernate.ScrollMode;
import org.hibernate.ScrollableResults;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.Transaction;
import org.hibernate.cfg.Configuration;
import askap.interfaces.skymodelservice.Component;

/**
 * Persistence interface class for the Sky Model Service.
 */
public class PersistenceInterface {
	/**
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(PersistenceInterface.class
			.getName());

	/**
	 * Size of batches for SQL inserts and updates
	 */
	private int itsBatchSize = 20;

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
	 * Perform a cone search.
	 *  
	 * @param ra	the right ascension of the centre of the
	 * 				search area (Units: decimal degrees).
	 * 				Range: 0 to 359.9...
	 * @param dec	the declination of the centre of the search
	 * 				area (Units: decimal degrees). Range: +/- 90
	 * @param searchRadius	the search radius (Units: decimal degrees).
	 * @param fluxLimit	low limit on flux on sources returned. All returned
	 * 					sources shall have flux >= fluxLimit (Units: Jy).
	 * @return	a list of component IDs for those components in the sky model
	 * 			which match the search criteria.
	 */
	public List<Long> coneSearch(double ra, double dec,
			double searchRadius, double fluxLimit) {
		ArrayList<Long> ids = new ArrayList<Long>();

		// TODO: Need to do the cone search in the query, instead of returning
		// components regardless of position.

		final String query = "from Component where i1400 >=" + fluxLimit;		
		ScrollableResults components = itsSession.createQuery(query).scroll(ScrollMode.FORWARD_ONLY);
		while (components.next()) {
			Component comp = (Component) components.get(0);
			assert(comp.i1400 >= fluxLimit);
			if (angularSeparation(ra, dec, comp) <= searchRadius) {
				ids.add(new Long(comp.id));
			}
		}

		return ids;
	}


	/**
	 * @param componentIds	a list of component IDs for which the component
	 * 						objects they identify are to be returned.
	 * @return a list of components.
	 */
	public List<Component> getComponents(List<Long> componentIds) {
		ArrayList<Component> components = new ArrayList<Component>();

		Iterator<Long> it = componentIds.iterator();
		while(it.hasNext()) {
			Component c = (Component) itsSession.get(Component.class, it.next());
			if (c != null) {
				components.add(c);
			}
		}

		return components;
	}


	/**
	 * @param components a list of components to add to the sky model.
	 * @return a list of component IDs, with index matching that of the
	 * "components" parameter passed as input.
	 */
	public List<Long> addComponents(List<Component> components) {
		ArrayList<Long> idList = new ArrayList<Long>();

		Iterator<Component> it = components.iterator();
		Transaction tx = itsSession.beginTransaction();
		int count = 0;
		while(it.hasNext()) {
			Component c = it.next();
			itsSession.save(c);
			idList.add(new Long(c.id));
			if (count == itsBatchSize) {
				itsSession.flush();
				itsSession.clear();
				count = 0;
			} else {
				count++;
			}
		}
		tx.commit();

		return idList;
	}


	/**
	 * @param componentIds	a vector of component IDs for which those
	 * 						components are to be removed from the sky model.
	 */
	public void removeComponents(List<Long> componentIds) {	
		Transaction tx = itsSession.beginTransaction();
		for (Long id : (List<Long>) componentIds) {
			Component c = (Component) itsSession.get(Component.class, id);
			if (c != null) {
				itsSession.delete(c);
			}
		}
		tx.commit();
	}

	/**
	 * Calculates the angular separation between a position given by ra/dec
	 * and a component instance.
	 * 
	 * @param ra	right ascension in decimal degrees (Range: 0 to 359.9...)
	 * @param dec	declination in decimal degrees (Range: +/-90)
	 * @param comp	component to determine angular separation for
	 * @return	the angular separation between the position specified by the
	 * 			parameters ra and dec, and the specified component.
	 */
	double angularSeparation(double ra, final double dec, final Component comp) {
		final double[] refXYZ = createXYZ(Math.toRadians(ra), Math.toRadians(dec));
		final double[] compXYZ = createXYZ(Math.toRadians(comp.rightAscension), Math.toRadians(comp.declination));

		final double d1 = Math.sqrt(square(refXYZ[0] - compXYZ[0]) +
				square(refXYZ[1] - compXYZ[1]) +
				square(refXYZ[2] - compXYZ[2]))/2.0;
		return Math.toDegrees(2 * Math.asin(d1 < 1.0 ? d1 : 1.0));
	}

	/**
	 * Computes the square of a number.
	 * @param val	the value to square.
	 * @return	the square of the "val" parameter.
	 */
	double square(final double val) {
		return Math.pow(val, 2);
	}

	/**
	 * For a given ra/dec creates a 3-vector of direction cosines,
	 * the Cartesian coordinates for the given position.
	 * @param ra	right ascension in decimal degrees (Range: 0 to 359.9...)
	 * @param dec	declination in decimal degrees (Range: +/-90)
	 * @return	the Cartesian coordinates, x, y & z. Where x points 
	 * 			to the prime meridian equator, z points to the north pole,  
	 * 			and y is normal to x and z.
	 */
	double[] createXYZ(final double ra, final double dec) {
		double[] xyz = new double[3];
		final double loc = Math.cos(dec);
		xyz[0] = Math.cos(ra) * loc;
		xyz[1] = Math.sin(ra) * loc;
		xyz[2] = Math.sin(dec);
		return xyz;
	}
}
