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
package askap.cp.calds.persist;

import static org.junit.Assert.*;

import java.sql.Connection;
import java.sql.DriverManager;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

// ASKAPsoft imports
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.cfg.Configuration;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import askap.interfaces.DoubleComplex;
import askap.interfaces.calparams.JonesIndex;
import askap.interfaces.calparams.JonesJTerm;
import askap.interfaces.calparams.TimeTaggedBandpassSolution;
import askap.interfaces.calparams.TimeTaggedGainSolution;
import askap.interfaces.calparams.TimeTaggedLeakageSolution;
import askap.interfaces.calparams.FrequencyDependentJTerm;

/**
 * 
 */
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
	askap.cp.calds.persist.PersistenceInterface itsInstance;
	
	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

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
		config.addClass(askap.cp.calds.persist.GainSolutionBean.class);
		config.addClass(askap.cp.calds.persist.GainSolutionElementBean.class);
		config.addClass(askap.cp.calds.persist.LeakageSolutionBean.class);
		config.addClass(askap.cp.calds.persist.LeakageSolutionElementBean.class);
		config.addClass(askap.cp.calds.persist.BandpassSolutionBean.class);
		config.addClass(askap.cp.calds.persist.BandpassSolutionElementBean.class);

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
	 * Test method for {@link askap.cp.calds.persist.PersistenceInterface#addGainSolution(askap.cp.calds.persist.TimeTaggedGainSolutionBean)}.
	 */
	@Test
	public void testAddGainSolution() {
		assertTrue(itsInstance.addGainSolution(createTestGainSolution(110)) != -1);
	}

	/**
	 * Test method for {@link askap.cp.calds.persist.PersistenceInterface#addLeakageSolution(askap.cp.calds.persist.TimeTaggedLeakageSolutionBean)}.
	 */
	@Test
	public void testAddLeakageSolution() {
		assertTrue(itsInstance.addLeakageSolution(cretaeTestLeakageSolution(111)) != -1);
	}

	/**
	 * Test method for {@link askap.cp.calds.persist.PersistenceInterface#addBandpassSolution(askap.cp.calds.persist.TimeTaggedBandpassSolutionBean)}.
	 */
	@Test
	public void testAddBandpassSolution() {
		assertTrue(itsInstance.addBandpassSolution(createTestBandpassSolution(112)) != -1);
	}
	
	@Test
	public void testGetGainSolution() {
		final long id = itsInstance.addGainSolution(createTestGainSolution(220));
		assertTrue(id != -1);
		
		TimeTaggedGainSolution sol = itsInstance.getGainSolution(id);
		assertNotNull(sol);
		
		// Check the timestamp
		assertEquals(220, sol.timestamp);
		
		// Check the gain member
		assertNotNull(sol.gain);
		
		// Get one JonesJTerm
		JonesIndex jind = new JonesIndex((short)1, (short)1);
		JonesJTerm jterm = sol.gain.get(jind);
		assertNotNull(jterm);
		assertEquals(new askap.interfaces.DoubleComplex(1.0, 1.0), jterm.g1);
		assertTrue(jterm.g1Valid);
		assertEquals(new askap.interfaces.DoubleComplex(2.0, 1.0), jterm.g2);
		assertFalse(jterm.g2Valid);
	}
	
	@Test
	public void testGetLeakageSolution() {
		final long id = itsInstance.addLeakageSolution(cretaeTestLeakageSolution(221));
		assertTrue(id != -1);
		
		TimeTaggedLeakageSolution sol = itsInstance.getLeakageSolution(id);
		assertNotNull(sol);
		
		// Check the timestamp
		assertEquals(221, sol.timestamp);
		
		// Check the leakage member
		assertNotNull(sol.leakage);
		
		JonesIndex jind = new JonesIndex((short)1, (short)1);
		DoubleComplex leakage = sol.leakage.get(jind);
		assertNotNull(leakage);
		assertEquals(1.0, leakage.real, 0.00001);
		assertEquals(1.0, leakage.imag, 0.00001);
		assertEquals(new askap.interfaces.DoubleComplex(1.0, 1.0), leakage);
	}
	
	@Test
	public void testGetBandpassSolution() {
		final long id = itsInstance.addBandpassSolution(createTestBandpassSolution(222));
		assertTrue(id != -1);
		
		TimeTaggedBandpassSolution sol = itsInstance.getBandpassSolution(id);
		assertNotNull(sol);
		
		// Check the timestamp
		assertEquals(222, sol.timestamp);
		
		// Check the bandpass member
		assertNotNull(sol.bandpass);	
		
		JonesIndex jind = new JonesIndex((short)1, (short)1);
		FrequencyDependentJTerm terms = sol.bandpass.get(jind);
		assertNotNull(terms.bandpass);
		
		// Should have one element in the list
		List<JonesJTerm> bandpass = terms.bandpass;
		assertEquals(1, bandpass.size());
		
		JonesJTerm jterm = bandpass.get(0);
		assertNotNull(jterm);
		
		assertEquals(new askap.interfaces.DoubleComplex(1.0, 1.0), jterm.g1);
		assertTrue(jterm.g1Valid);
		assertEquals(new askap.interfaces.DoubleComplex(2.0, 1.0), jterm.g2);
		assertFalse(jterm.g2Valid);
	}
	
	//////////////////////////////////////////////
	// Utility Methods
	//////////////////////////////////////////////
	
	TimeTaggedGainSolution createTestGainSolution(long timestamp) {
		TimeTaggedGainSolution solution = new TimeTaggedGainSolution();
		solution.timestamp = timestamp;
		
		// Flesh out the data structure
		JonesJTerm jterm = new JonesJTerm();
		jterm.g1 = new askap.interfaces.DoubleComplex(1.0, 1.0);
		jterm.g1Valid = true;
		jterm.g2 = new askap.interfaces.DoubleComplex(2.0, 1.0);
		jterm.g2Valid = false;
		JonesIndex jind = new JonesIndex((short)1, (short)1);
		solution.gain = new HashMap<JonesIndex, JonesJTerm>();
		solution.gain.put(jind, jterm);
		
		return solution;
	}

	TimeTaggedLeakageSolution cretaeTestLeakageSolution(long timestamp) {
		TimeTaggedLeakageSolution solution = new TimeTaggedLeakageSolution();
		solution.timestamp = timestamp;

		// Flesh out the data structure
		solution.leakage = new HashMap<JonesIndex, DoubleComplex>();
		solution.leakage.put(new JonesIndex((short)1, (short)1), 
				new DoubleComplex(1.0, 1.0));
		
		return solution;
	}

	TimeTaggedBandpassSolution createTestBandpassSolution(long timestamp) {
		TimeTaggedBandpassSolution solution = new TimeTaggedBandpassSolution();
		solution.timestamp = timestamp;
		solution.nChan = 1;
		
		// Flesh out the data structure
		solution.bandpass = new HashMap<JonesIndex, FrequencyDependentJTerm>();
		
		FrequencyDependentJTerm terms = new FrequencyDependentJTerm();
		terms.bandpass = new ArrayList<JonesJTerm>();
		
		// Add a JTerm to the list (for 1 channel)
		JonesJTerm jterm = new JonesJTerm();
		jterm.g1 = new askap.interfaces.DoubleComplex(1.0, 1.0);
		jterm.g1Valid = true;
		jterm.g2 = new askap.interfaces.DoubleComplex(2.0, 1.0);
		jterm.g2Valid = false;
		terms.bandpass.add(jterm);
		
		solution.bandpass.put(new JonesIndex((short)1, (short)1), terms);
		
		return solution;
	}
}
