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

// Java imports
import java.util.List;

// JUnit imports
import static org.junit.Assert.*;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

// ASKAPsoft imports
import java.sql.Connection;
import java.sql.DriverManager;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.cfg.Configuration;
import askap.cp.rfisourcesvc.persist.PersistenceInterface;
import askap.interfaces.rfisourcesvc.RFIEntry;

public class PersistenceInterfaceTest {

	/**
	 * Hibernate session object
	 */
	Session itsSession;
	
	/**
	 * HSQLDB connection, to interact directly with the database
	 */
	Connection itsConnection;
	
	/**
	 * Class under test.
	 */
	askap.cp.rfisourcesvc.persist.PersistenceInterface itsInstance;
	
	/**
	 * @throws java.lang.Exception
	 */
	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	/**
	 * @throws java.lang.Exception
	 */
	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		// First need to load the driver (not always necessary)
        try {
            Class.forName("org.hsqldb.jdbcDriver" );
        } catch (Exception e) {
            System.out.println("ERROR: failed to load HSQLDB JDBC driver.");
        }

		// HSQLDB connection, to interact directly with the database
		itsConnection = DriverManager.getConnection("jdbc:hsqldb:mem:aname;shutdown=true", "sa", "");
		
		// Configure Hibernate
		Configuration config = new Configuration();
		config.setProperty("hibernate.dialect", "org.hibernate.dialect.HSQLDialect");
		config.setProperty("hibernate.connection.driver_class", "org.hsqldb.jdbcDriver"); 
		config.setProperty("hibernate.connection.url", "jdbc:hsqldb:mem:baseball");
		config.setProperty("hibernate.connection.username", "sa");
		config.setProperty("hibernate.connection.password", "");
		config.setProperty("hibernate.connection.pool_size", "1"); 
		config.setProperty("hibernate.connection.autocommit", "true"); 
		config.setProperty("hibernate.cache.provider_class", "org.hibernate.cache.HashtableCacheProvider");
		config.setProperty("hibernate.hbm2ddl.auto", "create-drop");
		config.setProperty("hibernate.show_sql", "true");
		config.addClass(askap.interfaces.rfisourcesvc.RFIEntry.class);
		
		SessionFactory sessionFactory = config.buildSessionFactory();
		itsSession = sessionFactory.openSession();
		
		itsInstance = new PersistenceInterface(itsSession);
	}

	/**
	 * @throws java.lang.Exception
	 */
	@After
	public void tearDown() throws Exception {
		itsInstance = null;
		
		if (itsSession != null) {
			itsSession.close();
			itsSession = null;
		}
		
		// The database is shutdown when this connection closes
		if (itsConnection != null) {
			itsConnection.close();
			itsConnection = null;
		}
	}

	/**
	 * Test method for {@link askap.cp.rfisourcesvc.persist.PersistenceInterface#addEntry(askap.interfaces.rfisourcesvc.RFIEntry)}.
	 */
	@Test
	public final void testAddEntry() {
		// Add a component
		RFIEntry entry = createEntry();
		final long id1 = itsInstance.addEntry(entry);
		assertEquals(1, id1);
		
		// Add another one and ensure the id is different
		entry = createEntry();
		final long id2 = itsInstance.addEntry(entry);
		assertNotSame(id1, id2);
	}

	/**
	 * Test method for {@link askap.cp.rfisourcesvc.persist.PersistenceInterface#getAllEntries()}.
	 */
	@Test
	public final void testGetAllEntries() {
		// First verify the returned list is empty
		List<RFIEntry> list = itsInstance.getAllEntries();
		assertEquals(0, list.size());
		
		// Add a component
		RFIEntry entry = createEntry();
		itsInstance.addEntry(entry);
		
		// The returned list should now have one entry
		list = itsInstance.getAllEntries();
		assertEquals(1, list.size());
		
		// Ensure the entry looks as it should
		final double tolerance = 1e-15;
		assertEquals(1380 * 1000000, list.get(0).skyFrequency, tolerance);
		assertTrue(list.get(0).active);
		
		// Add another component
		entry = createEntry();
		itsInstance.addEntry(entry);
		
		// The returned list should now have two entries
		list = itsInstance.getAllEntries();
		assertEquals(2, list.size());
	}

	/**
	 * Test method for {@link askap.cp.rfisourcesvc.persist.PersistenceInterface#modifyEntry(askap.interfaces.rfisourcesvc.RFIEntry)}.
	 */
	@Test
	public final void testModifyEntry() {
		// Add a component
		RFIEntry entry = createEntry();
		final long id1 = itsInstance.addEntry(entry);
		assertEquals(1, id1);
		
		// Modify it
		entry.active = false;
		itsInstance.modifyEntry(entry);
	}
	
	private RFIEntry createEntry() {
		RFIEntry entry = new RFIEntry();
		entry.skyFrequency = 1380 * 1000000;
		entry.bandwidth = 300 * 1000000;
		entry.active = true;
		entry.dirSpecific = false;
		entry.azimuth = 0.0;
		entry.azimuthRange = 0.0;
		entry.elevation = 0.0;
		entry.elevationRange = 0.0;
		entry.timeSpecific = false;
		entry.startTime = 0;
		entry.endTime = 0;
		entry.source = "Unknown";
		entry.comments = "No comment";
		
		return entry;
	}
}
