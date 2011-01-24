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

// JUnit imports
import static org.junit.Assert.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

// Support classes
import askap.cp.manager.rman.JobTemplate;

public class QResourceManagerTest {

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	/*
	 * Verify the dependency argument can be built from the
	 * dependency infomation in the JobTemplate.
	 */
	@Test
	public void testBuildDependencyArg() {
		JobTemplate template = new JobTemplate("testjob");
		QResourceManager rm = new QResourceManager();

		// Test with no dependencies
		assertEquals("", rm.buildDependencyArg(template));

		// Add 1234 as an afterok dependency
		QJob j1 = new QJob("1234");
		template.addDependency(j1, JobTemplate.DependType.AFTEROK);

		assertEquals("-W depend=afterok:1234", rm.buildDependencyArg(template));
		
		// Add 5678 as an after dependency
		QJob j2 = new QJob("5678");
		template.addDependency(j2, JobTemplate.DependType.AFTERSTART);
		assertEquals("-W depend=afterok:1234,after:5678", rm.buildDependencyArg(template));
	}
}