/*
 * Copyright (c) 2009 CSIRO Australia Telescope National Facility (ATNF) Commonwealth
 * Scientific and Industrial Research Organisation (CSIRO) PO Box 76, Epping NSW 1710,
 * Australia atnf-enquiries@csiro.au
 * 
 * This file is part of the ASKAP software distribution.
 * 
 * The ASKAP software distribution is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this
 * program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite
 * 330, Boston, MA 02111-1307 USA
 */

package askap.util;

import org.junit.*;
import static org.junit.Assert.*;
import askap.util.ParameterSet;

/**
 * Performs JUnit Tests for ParameterSet class.
 * @author David Brodrick
 */
public class ParameterSetTest
{
    private static ParameterSet theirTestSet;

    static {
        // Create the ParameterSet
        theirTestSet = new ParameterSet();
        theirTestSet.put("float", "3.141");
        theirTestSet.put("string", "foo");
        theirTestSet.put("integer", "1234");
        theirTestSet.put("boolean", "true");
        theirTestSet.put("prefix.suffix", "bar");
    }

    @Test
    public void test_String()
    {
        assertTrue(theirTestSet.getString("string").equals("foo"));
    }

    @Test
    public void test_StringDefault()
    {
        assertTrue(theirTestSet.getString("string", "bar").equals("foo"));
    }

    @Test
    public void test_StringDefaultMissing()
    {
        assertTrue(theirTestSet.getString("dummy", "bar").equals("bar"));
    }

    @Test
    public void test_Float()
    {
        assertTrue(theirTestSet.getFloat("float") == 3.141f);
    }

    @Test
    public void test_FloatDefault()
    {
        assertTrue(theirTestSet.getFloat("float", 2.71f) == 3.141f);
    }

    @Test
    public void test_FloatDefaultMissing()
    {
        assertTrue(theirTestSet.getFloat("dummy", 2.71f) == 2.71f);
    }

    @Test
    public void test_FloatBad()
    {
        try {
            theirTestSet.getFloat("string");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_FloatMissing()
    {
        try {
            theirTestSet.getFloat("dummy");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_FloatDefaultBad()
    {
        try {
            theirTestSet.getFloat("string", 2.71f);
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_Double()
    {
        assertTrue(theirTestSet.getDouble("float") == 3.141);
    }

    @Test
    public void test_DoubleDefault()
    {
        assertTrue(theirTestSet.getDouble("float", 2.71) == 3.141);
    }

    @Test
    public void test_DoubleDefaultMissing()
    {
        assertTrue(theirTestSet.getDouble("dummy", 2.71) == 2.71);
    }

    @Test
    public void test_DoubleBad()
    {
        try {
            theirTestSet.getDouble("string");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_DoubleMissing()
    {
        try {
            theirTestSet.getDouble("dummy");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_DoubleDefaultBad()
    {
        try {
            theirTestSet.getDouble("string", 2.71);
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_Integer()
    {
        assertTrue(theirTestSet.getInteger("integer") == 1234);
    }

    @Test
    public void test_IntegerDefault()
    {
        assertTrue(theirTestSet.getInteger("integer", 4321) == 1234);
    }

    @Test
    public void test_IntegerDefaultMissing()
    {
        assertTrue(theirTestSet.getInteger("dummy", 4321) == 4321);
    }

    @Test
    public void test_IntegerBad()
    {
        try {
            theirTestSet.getInteger("string");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_IntegerMissing()
    {
        try {
            theirTestSet.getInteger("dummy");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_IntegerDefaultBad()
    {
        try {
            theirTestSet.getInteger("string", 4321);
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_Long()
    {
        assertTrue(theirTestSet.getLong("integer") == 1234);
    }

    @Test
    public void test_LongDefault()
    {
        assertTrue(theirTestSet.getLong("integer", 4321) == 1234);
    }

    @Test
    public void test_LongDefaultMissing()
    {
        assertTrue(theirTestSet.getLong("dummy", 4321) == 4321);
    }

    @Test
    public void test_LongBad()
    {
        try {
            theirTestSet.getLong("string");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_LongMissing()
    {
        try {
            theirTestSet.getLong("dummy");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_LongDefaultBad()
    {
        try {
            theirTestSet.getLong("string", 4321);
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_Boolean()
    {
        assertTrue(theirTestSet.getBoolean("boolean"));
    }

    @Test
    public void test_BooleanDefault()
    {
        assertTrue(theirTestSet.getBoolean("boolean", false));
    }

    @Test
    public void test_BooleanDefaultMissing()
    {
        assertTrue(theirTestSet.getBoolean("dummy", true));
    }

    @Test
    public void test_BooleanMissing()
    {
        try {
            theirTestSet.getBoolean("dummy");
            assertTrue(false);
        } catch (NumberFormatException e) {
        }
    }

    @Test
    public void test_Subset()
    {
        ParameterSet subset = theirTestSet.subset("prefix.");
        assertTrue(subset.size()==1);
        assertTrue(subset.getString("suffix")!=null);
        assertTrue(subset.getString("suffix").equals("bar"));
    }

    @Test
    public void test_SubsetEmpty()
    {
        ParameterSet subset = theirTestSet.subset("dummy");
        assertTrue(subset.size()==0);
    }
}
