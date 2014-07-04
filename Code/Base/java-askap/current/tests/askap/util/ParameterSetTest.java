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
 * 
 * @author David Brodrick
 */
public class ParameterSetTest {
    private static final float FLOAT_DELTA = 1.0e-15f;
    private ParameterSet itsInstance;

    @Before
    public void setUp() throws Exception {
        itsInstance = new ParameterSet();
        itsInstance.add("float", "3.141");
        itsInstance.add("string", "foo");
        itsInstance.add("integer", "1234");
        itsInstance.add("boolean", "true");
        itsInstance.add("prefix.suffix", "bar");

        itsInstance.add("short", "[5, 6, 8, 10]");
        itsInstance.add("time", "[]");
    }
    
    @After
    public void tearDown() throws Exception {
        itsInstance = null;
    }

    @Test
    public void test_SimpleTypedObject() {
        assertEquals("float type", new Float(3.141f),
                (Float) itsInstance.getObject("float", "java.lang.Float"));
        assertEquals("string type", "foo",
                itsInstance.getObject("string", null));
        assertEquals("integer type", 1234,
                itsInstance.getObject("integer", "java.lang.Integer"));
        assertTrue("boolean type", ((Boolean) itsInstance.getObject("boolean",
                "java.lang.Boolean")).booleanValue());
    }

    @Test
    public void test_ArrayObject() {
        Object shortArray[] = (Object[]) itsInstance.getVectorValue("short",
                "java.lang.Short");

        assertArrayEquals("short array", new Object[] { new Short("5"),
                new Short("6"), new Short("8"), new Short("10") }, shortArray);

        Object byteArray[] = (Object[]) itsInstance.getVectorValue("time",
                "java.lang.Long");
        assertEquals("string array should have 0 element", 0, byteArray.length);
    }

    @Test
    public void test_ObjectToString() {
        Integer intArray[] = new Integer[5];
        intArray[0] = new Integer(1);
        intArray[1] = new Integer(5);
        intArray[2] = new Integer(10);
        intArray[3] = new Integer(55);
        intArray[4] = new Integer(80);

        assertEquals("[1,5,10,55,80]", ParameterSet.getStrValue(intArray));
        assertEquals("[]", ParameterSet.getStrValue(new Double[0]));
    }

    @Test
    public void test_Expandable() {
        // [3*3*2] ==> [2,2,2,2,2,2,2,2,2]
        Object intArray[] = ParameterSet.getVector("[3*3*2]",
                "java.lang.Integer");
        assertEquals("incorrect short array count", 9, intArray.length);
        for (int i = 0; i < intArray.length; i++) {
            int val = (Integer) intArray[i];
            assertEquals("strArray[" + i + "] should be 2", 2, val);
        }

        // [3*'2*3'] ==> ['2*3','2*3','2*3']
        Object strArray[] = ParameterSet.getVector("[3*'2*3']",
                "java.lang.String");
        assertEquals(3, strArray.length);
        assertEquals("array[0]", "2*3", strArray[0]);
        assertEquals("array[1]", "2*3", strArray[1]);
        assertEquals("array[2]", "2*3", strArray[2]);

        // [3*ab] ==> [ab,ab,ab]
        strArray = ParameterSet.getVector("[3*ab]", "java.lang.String");
        assertEquals("incorrect str array count", 3, strArray.length);
        for (int i = 0; i < strArray.length; i++) {
            String s = (String) strArray[1];
            assertEquals("strArray[" + i + "] should be ab", "ab", s);
        }

        // [2*3*ab] ==> [ab,ab,ab,ab,ab,ab]
        strArray = ParameterSet.getVector("[2*3*ab]", "java.lang.String");
        assertEquals("incorrect str array count", 6, strArray.length);
        for (int i = 0; i < strArray.length; i++) {
            String s = (String) strArray[1];
            assertEquals("strArray[" + i + "] should be ab", "ab", s);
        }

        // [3*10,5*2] ==> [10,10,10,2,2,2,2,2]
        intArray = ParameterSet.getVector("[3*10,5*2]", "java.lang.Integer");
        assertEquals("incorrect array count", 8, intArray.length);
        assertEquals("array[0]", 10, intArray[0]);
        assertEquals("array[1]", 10, intArray[1]);
        assertEquals("array[2]", 10, intArray[2]);
        assertEquals("array[3]", 2, intArray[3]);
        assertEquals("array[4]", 2, intArray[4]);
        assertEquals("array[5]", 2, intArray[5]);
        assertEquals("array[6]", 2, intArray[6]);
        assertEquals("array[7]", 2, intArray[7]);

        // [3*(1,2,3,4)] ==> [1,2,3,4,1,2,3,4,1,2,3,4]
        intArray = ParameterSet.getVector("[3*(1,2,3,4)]", "java.lang.Integer");
        assertEquals("incorrect array count", 12, intArray.length);
        assertEquals("array[0]", 1, intArray[0]);
        assertEquals("array[1]", 2, intArray[1]);
        assertEquals("array[2]", 3, intArray[2]);
        assertEquals("array[3]", 4, intArray[3]);
        assertEquals("array[4]", 1, intArray[4]);
        assertEquals("array[5]", 2, intArray[5]);
        assertEquals("array[6]", 3, intArray[6]);
        assertEquals("array[7]", 4, intArray[7]);
        assertEquals("array[8]", 1, intArray[8]);
        assertEquals("array[9]", 2, intArray[9]);
        assertEquals("array[10]", 3, intArray[10]);
        assertEquals("array[11]", 4, intArray[11]);

        // [3 * 1 .. 4] ==> [1,2,3,4,1,2,3,4,1,2,3,4]
        intArray = ParameterSet.getVector("[3 * 1 .. 4]", "java.lang.Integer");
        assertEquals("incorrect array count", 12, intArray.length);
        assertEquals("array[0]", 1, intArray[0]);
        assertEquals("array[1]", 2, intArray[1]);
        assertEquals("array[2]", 3, intArray[2]);
        assertEquals("array[3]", 4, intArray[3]);
        assertEquals("array[4]", 1, intArray[4]);
        assertEquals("array[5]", 2, intArray[5]);
        assertEquals("array[6]", 3, intArray[6]);
        assertEquals("array[7]", 4, intArray[7]);
        assertEquals("array[8]", 1, intArray[8]);
        assertEquals("array[9]", 2, intArray[9]);
        assertEquals("array[10]", 3, intArray[10]);
        assertEquals("array[11]", 4, intArray[11]);

        // NESTED ARRAY IS NOT SUPPORTED
        // [2*[[1,2,3],[4,5,6]]] ==> [[[1,2,3],[4,5,6]],[[1,2,3],[4,5,6]]]

        // [3*'10.5*ab'] ==> ['10.5*ab','10.5*ab','10.5*ab']
        strArray = ParameterSet.getVector("[3*'10.5*ab']", "java.lang.String");
        assertEquals("incorrect array count", 3, strArray.length);
        assertEquals("array[0]", "10.5*ab", strArray[0]);
        assertEquals("array[1]", "10.5*ab", strArray[1]);
        assertEquals("array[2]", "10.5*ab", strArray[2]);

        // [10.5*'ab'] ==> [10.5*'ab']
        strArray = ParameterSet.getVector("[10.5*'ab']", "java.lang.String");
        assertEquals("incorrect array count", 1, strArray.length);
        assertEquals("array[0]", "10.5*'ab'", strArray[0]);

        // [3*10.5*'ab'] ==> [10.5*'ab',10.5*'ab',10.5*'ab']
        strArray = ParameterSet.getVector("[3*10.5*'ab']", "java.lang.String");
        assertEquals("incorrect array count", 3, strArray.length);
        assertEquals("array[0]", "10.5*'ab'", strArray[0]);
        assertEquals("array[1]", "10.5*'ab'", strArray[1]);
        assertEquals("array[2]", "10.5*'ab'", strArray[2]);

        // [3*'ab'*2] ==> ['ab'*2,'ab'*2,'ab'*2]
        strArray = ParameterSet.getVector("[3*'ab'*2]", "java.lang.String");
        assertEquals("incorrect array count", 3, strArray.length);
        assertEquals("array[0]", "'ab'*2", strArray[0]);
        assertEquals("array[1]", "'ab'*2", strArray[1]);
        assertEquals("array[2]", "'ab'*2", strArray[2]);

        // [3*ab*2] ==> [ab*2,ab*2,ab*2]
        strArray = ParameterSet.getVector("[3*ab*2]", "java.lang.String");
        assertEquals("incorrect array count", 3, strArray.length);
        assertEquals("array[0]", "ab*2", strArray[0]);
        assertEquals("array[1]", "ab*2", strArray[1]);
        assertEquals("array[2]", "ab*2", strArray[2]);

        // [1*(1,2,3)] ==> [1,2,3]
        intArray = ParameterSet.getVector("[1*(1,2,3)]", "java.lang.Integer");
        assertEquals("incorrect array count", 3, intArray.length);
        assertEquals("array[0]", 1, intArray[0]);
        assertEquals("array[1]", 2, intArray[1]);
        assertEquals("array[2]", 3, intArray[2]);

        // [(1,2,3)] ==> [(1,2,3)]
        strArray = ParameterSet.getVector("[(1,2,3)]", "java.lang.String");
        assertEquals("incorrect array count", 3, strArray.length);
        assertEquals("array[0]", "(1", strArray[0]);
        assertEquals("array[1]", "2", strArray[1]);
        assertEquals("array[2]", "3)", strArray[2]);

        // [ab013..010] ==> [ab013,ab012,ab011,ab010]
        strArray = ParameterSet.getVector("[ab013..010]", "java.lang.String");
        assertEquals("incorrect array count", 4, strArray.length);
        assertEquals("array[0]", "ab013", strArray[0]);
        assertEquals("array[1]", "ab012", strArray[1]);
        assertEquals("array[2]", "ab011", strArray[2]);
        assertEquals("array[3]", "ab010", strArray[3]);

        // [/aa000..2] ==> [/aa000,/aa001,/aa002]
        strArray = ParameterSet.getVector("[/aa000..2]", "java.lang.String");
        assertEquals("incorrect array count", 3, strArray.length);
        assertEquals("array[0]", "/aa000", strArray[0]);
        assertEquals("array[1]", "/aa001", strArray[1]);
        assertEquals("array[2]", "/aa002", strArray[2]);

        // [/aa000../aa2] ==> [/aa000,/aa001,/aa002]
        strArray = ParameterSet.getVector("[/aa000../aa2]", "java.lang.String");
        assertEquals("incorrect array count", 3, strArray.length);
        assertEquals("array[0]", "/aa000", strArray[0]);
        assertEquals("array[1]", "/aa001", strArray[1]);
        assertEquals("array[2]", "/aa002", strArray[2]);
    }

    @Test
    public void test_String() {
        assertEquals("foo", itsInstance.getString("string"));
    }

    @Test
    public void test_StringDefault() {
        assertEquals("foo", itsInstance.getString("string", "bar"));
    }

    @Test
    public void test_StringDefaultMissing() {
        assertEquals("bar", itsInstance.getString("dummy", "bar"));
    }

    @Test
    public void test_Float() {
        assertEquals(3.141f, itsInstance.getFloat("float"), FLOAT_DELTA);
    }

    @Test
    public void test_FloatDefault() {
        assertEquals(3.141f, itsInstance.getFloat("float", 2.71f), FLOAT_DELTA);
    }

    @Test
    public void test_FloatDefaultMissing() {
        assertEquals(2.71f, itsInstance.getFloat("dummy", 2.71f), FLOAT_DELTA);
    }

    @Test(expected=NumberFormatException.class)
    public void test_FloatBad() {
        itsInstance.getFloat("string");
    }

    @Test(expected=NumberFormatException.class)
    public void test_FloatMissing() {
        itsInstance.getFloat("dummy");
    }

    @Test(expected=NumberFormatException.class)
    public void test_FloatDefaultBad() {
        itsInstance.getFloat("string", 2.71f);
    }

    @Test
    public void test_Double() {
        assertEquals(3.141, itsInstance.getDouble("float"), FLOAT_DELTA);
    }

    @Test
    public void test_DoubleDefault() {
        assertEquals(3.141, itsInstance.getDouble("float", 2.71), FLOAT_DELTA);
    }

    @Test
    public void test_DoubleDefaultMissing() {
        assertEquals(2.71, itsInstance.getDouble("dummy", 2.71), FLOAT_DELTA);
    }

    @Test(expected=NumberFormatException.class)
    public void test_DoubleBad() {
        itsInstance.getDouble("string");
    }

    @Test(expected=NumberFormatException.class)
    public void test_DoubleMissing() {
        itsInstance.getDouble("dummy");
    }

    @Test(expected=NumberFormatException.class)
    public void test_DoubleDefaultBad() {
        itsInstance.getDouble("string", 2.71);
    }

    @Test
    public void test_Integer() {
        assertEquals(1234, itsInstance.getInteger("integer"));
    }

    @Test
    public void test_IntegerDefault() {
        assertEquals(1234, itsInstance.getInteger("integer", 4321));
    }

    @Test
    public void test_IntegerDefaultMissing() {
        assertEquals(4321, itsInstance.getInteger("dummy", 4321));
    }

    @Test(expected=NumberFormatException.class)
    public void test_IntegerBad() {
        itsInstance.getInteger("string");
    }

    @Test(expected=NumberFormatException.class)
    public void test_IntegerMissing() {
            itsInstance.getInteger("dummy");
    }

    @Test(expected=NumberFormatException.class)
    public void test_IntegerDefaultBad() {
        itsInstance.getInteger("string", 4321);
    }

    @Test
    public void test_Long() {
        assertEquals(1234, itsInstance.getLong("integer"));
    }

    @Test
    public void test_LongDefault() {
        assertEquals(1234, itsInstance.getLong("integer", 4321));
    }

    @Test
    public void test_LongDefaultMissing() {
        assertEquals(4321, itsInstance.getLong("dummy", 4321));
    }

    @Test(expected=NumberFormatException.class)
    public void test_LongBad() {
        itsInstance.getLong("string");
    }

    @Test(expected=NumberFormatException.class)
    public void test_LongMissing() {
        itsInstance.getLong("dummy");
    }

    @Test(expected=NumberFormatException.class)
    public void test_LongDefaultBad() {
        itsInstance.getLong("string", 4321);
    }

    @Test
    public void test_Boolean() {
        assertTrue(itsInstance.getBoolean("boolean"));
    }

    @Test
    public void test_BooleanDefault() {
        assertTrue(itsInstance.getBoolean("boolean", false));
    }

    @Test
    public void test_BooleanDefaultMissing() {
        assertTrue(itsInstance.getBoolean("dummy", true));
    }

    @Test(expected=NumberFormatException.class)
    public void test_BooleanMissing() {
        itsInstance.getBoolean("dummy");
    }

    @Test
    public void test_Subset() {
        ParameterSet subset = itsInstance.subset("prefix.");
        assertEquals(1, subset.size());
        assertNotNull(subset.getString("suffix"));
        assertTrue(subset.getString("suffix").equals("bar"));
    }

    @Test
    public void test_SubsetEmpty() {
        ParameterSet subset = itsInstance.subset("dummy");
        assertEquals(0, subset.size());
    }
}
