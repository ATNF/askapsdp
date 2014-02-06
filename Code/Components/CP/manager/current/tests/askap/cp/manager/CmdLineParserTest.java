/**
 *  Copyright (c) 2014 CSIRO - Australia Telescope National Facility (ATNF)
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
package askap.cp.manager;

import static org.junit.Assert.*;

import java.util.Map;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class CmdLineParserTest {
	CmdLineParser instance;

	@Before
	public void setUp() throws Exception {
		instance = new CmdLineParser();
		instance.addOption("config", "c", "configuration parameter set file", true);
		instance.addOption("log-config", "l", "logger configuration file", true);
	}

	@After
	public void tearDown() throws Exception {
		instance = null;
	}

	@Test
	public void testParse() {
		String[] args = { "-c", "config.in", "--log-config", "logconfig.xml" };
		
		Map<String, String> options = instance.parse(args);
		assertTrue(options.containsKey("config"));
		assertEquals("config.in", options.get("config"));
		assertTrue(options.containsKey("log-config"));
		assertEquals("logconfig.xml", options.get("log-config"));
	}
	
	@Test(expected=RuntimeException.class)
	public void testParseMissingExpectedValue() {
		String[] args = { "-c"};
		instance.parse(args);
	}
}
