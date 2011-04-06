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
import java.util.List;

import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.cfg.Configuration;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import askap.interfaces.calparams.TimeTaggedGainSolution;

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
		config.addClass(askap.cp.calds.persist.TimeTaggedGainSolutionBean.class);
		
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
		// Add a Gain Solution
		TimeTaggedGainSolution solution = new TimeTaggedGainSolution();
		solution.timestamp = 123;
		itsInstance.addGainSolution(solution);
	}

	/**
	 * Test method for {@link askap.cp.calds.persist.PersistenceInterface#addLeakageSolution(askap.cp.calds.persist.TimeTaggedLeakageSolutionBean)}.
	 */
	@Test
	public void testAddLeakageSolution() {
		//fail("Not yet implemented");
	}

	/**
	 * Test method for {@link askap.cp.calds.persist.PersistenceInterface#addBandpassSolution(askap.cp.calds.persist.TimeTaggedBandpassSolutionBean)}.
	 */
	@Test
	public void testAddBandpassSolution() {
		//fail("Not yet implemented");
	}

}
