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
import java.sql.SQLException;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.cfg.Configuration;
import askap.cp.skymodelsvc.persist.PersistenceInterface;
import askap.interfaces.skymodelservice.Component;

public class PersistenceInterfaceTest {
	
	String itsComponentsTbl = "components";
	
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
	askap.cp.skymodelsvc.persist.PersistenceInterface itsInstance;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

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
		config.addClass(askap.interfaces.skymodelservice.Component.class);
		
		SessionFactory sessionFactory = config.buildSessionFactory();
		itsSession = sessionFactory.openSession();
		
		itsInstance = new PersistenceInterface(itsSession);
	}

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

	@Test
	public final void testAddComponents() throws SQLException {
		// Add a component
		Component c = new Component();
		ArrayList<Component> components = new ArrayList<Component>();
		components.add(c);
		List<Long> ids1 = itsInstance.addComponents(components);
		assertEquals(1, ids1.size());
		
		// Add another component and ensure the id differs from the first
		c = new Component();
		components.clear();
		components.add(c);
		List<Long> ids2 = itsInstance.addComponents(components);
		assertEquals(1, ids2.size());

		assertNotSame(ids1.get(0).longValue(), ids2.get(0).longValue());
		
		// Add multiple components
		components.clear();
		final int n = 10;
		for (int i = 0; i < n; i++) {
			c = new Component();
			components.add(c);
		}
		
		List<Long> ids3 = itsInstance.addComponents(components);
		assertEquals(n, ids3.size());
		assertNotSame(ids3.get(0).longValue(), ids3.get(1).longValue());
	}
	
	@Test
	public final void testGetComponents() {
		// Add some components
		Component c = new Component();
		final double ra = 90.0;
		final double dec = -40.0;
		
		c.rightAscension = ra;
		c.declination = dec;
		ArrayList<Component> components = new ArrayList<Component>();
		final int n = 10;
		for (int i = 0; i < n; i++) {
			components.add(c);
		}
		
		List<Long> ids = itsInstance.addComponents(components);
		assertEquals(n, ids.size());
		
		// Now get those components back
		List<Component> returnedComponents = itsInstance.getComponents(ids);
		assertEquals(n, returnedComponents.size());
		final double tolerance = 0.0000001;
		for (int i = 0; i < n; i++) {
			assertEquals(ra, returnedComponents.get(i).rightAscension, tolerance);
			assertEquals(dec, returnedComponents.get(i).declination, tolerance);
		}
	}

	@Test
	public final void testRemoveComponents() {
		// Add a component
		Component c = new Component();
		ArrayList<Component> components = new ArrayList<Component>();
		components.add(c);
		List<Long> ids1 = itsInstance.addComponents(components);
		assertEquals(1, ids1.size());
		
		// Add another component (using a new list)
		c = new Component();
		components.clear();
		components.add(c);
		List<Long> ids2 = itsInstance.addComponents(components);
		assertEquals(1, ids2.size());
		
		// Remove the first inserted component
		itsInstance.removeComponents(ids1);
		
		// Now get the first inserted component back, the list should be empty
		List<Component> returnedComponents = itsInstance.getComponents(ids1);
		assertEquals(0, returnedComponents.size());
		
		// Now get the second inserted component back
		returnedComponents = itsInstance.getComponents(ids2);
		assertEquals(1, returnedComponents.size());
		
		// Remove the second inserted component
		itsInstance.removeComponents(ids2);
		
		// Now get the second inserted component back, the list should be empty
		returnedComponents = itsInstance.getComponents(ids2);
		assertEquals(0, returnedComponents.size());
	}
	
	@Test
	public final void testConeSearch() {
		// Create a component
		Component c = new Component();
		c.rightAscension = 90.0;
		c.declination = -40.0;
		c.i1400 = 0.001; // 1mJy
		ArrayList<Component> components = new ArrayList<Component>();
		
		// Add a component
		components.add(c);
		List<Long> ids1 = itsInstance.addComponents(components);
		assertEquals(1, ids1.size());
		
		// Run a cone search, expecting to find the component
		List<Long> results = itsInstance.coneSearch(90.0, -40.0, 1.0, 0.00001);
		assertEquals(1, results.size());
		
		// Run a cone search, expecting NOT to find the the component due to
		// it being outside the search radius yet within the flux limits
		List<Long> resultsNone1 = itsInstance.coneSearch(0.0, 0.0, 1.0, 0.00001);
		assertEquals(0, resultsNone1.size());
		
		// Run a cone search, expecting NOT to find the the component due to
		// it being outside flux limit yet inside the search radius
		List<Long> resultsNone2 = itsInstance.coneSearch(90.0, -40.0, 1.0, 0.1);
		assertEquals(0, resultsNone2.size());
	}
	
	@Test
	public final void testAngularSeparation() {
		Component c = new Component();
		
		// NOTE: Expected results are based on those results the casa::MVidrection
		// class produces
		
		// Use an arc second tolerance. Should be sufficient for cone search
		final double tolerance = 1.0 / 60 / 60;
		
		// Ref and component at same location
		c.rightAscension = 1.0;
		c.declination = -45.0;
		assertEquals(0.0, PersistenceInterface.angularSeparation(c.rightAscension,
				c.declination, c), tolerance);
		
		// Test RA wraparound
		c.rightAscension = 1.0;
		c.declination = -45.0;
		assertEquals(0.777811, PersistenceInterface.angularSeparation(359.9, -45.0, c), tolerance);
		
		// Test Dec (Pole) wraparound
		c.rightAscension = 187.5;
		c.declination = -89.9;
		assertEquals(0.2, PersistenceInterface.angularSeparation(7.5, -89.9, c), tolerance);
		
		// Test far apart in RA (Range 0-180 degrees)
		c.rightAscension = 1.0;
		c.declination = 0.0;
		assertEquals(180.0, PersistenceInterface.angularSeparation(181.0, 0.0, c), tolerance);
		
		// Test far apart in RA (Range 181-360 degrees)
		c.rightAscension = 190.0;
		c.declination = 0.0;
		assertEquals(160.0, PersistenceInterface.angularSeparation(350.0, 0.0, c), tolerance);
		
		// Test far apart in Dec 
		c.rightAscension = 150.0;
		c.declination = 80.0;
		assertEquals(160, PersistenceInterface.angularSeparation(150.0, -80.0, c), tolerance);
	}
}
