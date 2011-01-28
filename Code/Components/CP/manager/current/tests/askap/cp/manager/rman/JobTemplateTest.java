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
package askap.cp.manager.rman;

import static org.junit.Assert.*;

import java.util.Map;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import askap.cp.manager.rman.JobTemplate.DependType;

/**
 * @author Ben Humphreys <ben.humphreys@csiro.au>
 *
 */
public class JobTemplateTest {
	private JobTemplate t = null;
	
	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		t = new JobTemplate("");
	}

	/**
	 * @throws java.lang.Exception
	 */
	@After
	public void tearDown() throws Exception {
		t = null;
	}

	@Test
	public void testName() {
		final String testVal = "testjob";
		t.setName(testVal);
		assertEquals(testVal, t.getName());
	}

	/**
	 * Test method for {@link askap.cp.manager.rman.JobTemplate#setScriptLocation(java.lang.String)}.
	 */
	@Test
	public void testScriptLocation() {
		final String testVal = "/path/to/software/script.qsub";
		t.setScriptLocation(testVal);
		assertEquals(testVal, t.getScriptLocation());
	}

	/**
	 * Test method for {@link askap.cp.manager.rman.JobTemplate#addDependency(askap.cp.manager.rman.IJob, askap.cp.manager.rman.JobTemplate.DependType)}.
	 */
	@Test
	public void testAddDependency() {
		// Just check no exceptions are thrown
		QJob j1 = new QJob("1");
		t.addDependency(j1, JobTemplate.DependType.AFTERSTART);
		
		QJob j2 = new QJob("2");
		t.addDependency(j2, JobTemplate.DependType.AFTEROK);
	}

	/**
	 * Test method for {@link askap.cp.manager.rman.JobTemplate#removeDependency(askap.cp.manager.rman.IJob)}.
	 */
	@Test
	public void testRemoveDependency() {
		// Just check no exceptions are thrown
		QJob j1 = new QJob("1");
		t.addDependency(j1, JobTemplate.DependType.AFTERSTART);
		
		t.removeDependency(j1);
	}

	/**
	 * Test method for {@link askap.cp.manager.rman.JobTemplate#getDependencies()}.
	 */
	@Test
	public void testGetDependencies() {
		QJob j1 = new QJob("1");
		t.addDependency(j1, JobTemplate.DependType.AFTERSTART);
		
		QJob j2 = new QJob("2");
		t.addDependency(j2, JobTemplate.DependType.AFTEROK);
		
		Map<IJob, DependType> deps = t.getDependencies();
		assertEquals(2, deps.size());
		
		assertTrue(deps.containsKey(j1));
		assertTrue(deps.containsKey(j2));
	}

	/**
	 * Test method for {@link askap.cp.manager.rman.JobTemplate#removeAllDependencies()}.
	 */
	@Test
	public void testRemoveAllDependencies() {
		QJob j1 = new QJob("1");
		t.addDependency(j1, JobTemplate.DependType.AFTERSTART);
		
		QJob j2 = new QJob("2");
		t.addDependency(j2, JobTemplate.DependType.AFTEROK);
		
		Map<IJob, DependType> deps = t.getDependencies();
		assertEquals(2, deps.size());
		
		t.removeAllDependencies();
		
		deps = t.getDependencies();
		assertEquals(0, deps.size());
	}

}
