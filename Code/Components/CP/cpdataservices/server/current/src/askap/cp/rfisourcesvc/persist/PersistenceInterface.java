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

package askap.cp.rfisourcesvc.persist;

import java.util.List;

import org.apache.log4j.Logger;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.Transaction;
import org.hibernate.cfg.Configuration;

import askap.interfaces.rfisourcesvc.RFIEntry;

public class PersistenceInterface {
	/**
	 * Logger
	 */
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
		config.configure("rfisource-hibernate.cfg.xml");
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
	
	public long addEntry(RFIEntry entry) {
		Transaction tx = itsSession.beginTransaction();
		itsSession.save(entry);
		tx.commit();
		
		return entry.id;
	}
	
	public List<RFIEntry> getAllEntries() {
		Transaction tx = itsSession.beginTransaction();
		@SuppressWarnings("unchecked")
		List<RFIEntry> result = (List<RFIEntry>) itsSession.createQuery("from RFIEntry").list();
		tx.commit();
		
		return result;
	}
	
	public void modifyEntry(RFIEntry entry) {
		Transaction tx = itsSession.beginTransaction();
		itsSession.merge(entry);
		tx.commit();
	}
}
