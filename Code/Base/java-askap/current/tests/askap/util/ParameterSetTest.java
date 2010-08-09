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
        
        theirTestSet.put("short", "[5, 6, 8, 10]");
        theirTestSet.put("time", "[]");
        
        
    }
    @Test
    public void test_SimpleTypedObject() {
    	assertEquals("float type", new Float(3.141f), (Float)theirTestSet.getObject("float", "java.lang.Float"));
    	assertEquals("string type", "foo", theirTestSet.getObject("string", null));
    	assertEquals("integer type", 1234, theirTestSet.getObject("integer", "java.lang.Integer"));
    	assertTrue("boolean type", ((Boolean)theirTestSet.getObject("boolean", "java.lang.Boolean")).booleanValue());
    }
    
    @Test
    public void test_ArrayObject() {
    	Object shortArray[] = (Object[]) theirTestSet.getVectorValue("short", "java.lang.Short");
    	
    	assertArrayEquals("short array", new Object[]{new Short("5"), new Short("6"), new Short("8"), new Short("10")}, shortArray);
  	
    	Object byteArray[] = (Object[]) theirTestSet.getVectorValue("time", "java.lang.Long");
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
    	
    	assertEquals("convert to string", "[1,5,10,55,80]", ParameterSet.getStrValue(intArray));    	
    	
    	assertEquals("should convert to []", "[]", ParameterSet.getStrValue(new Double[0]));    	
    }
    
    @Test
    public void test_Expandable() {
    	// [3*3*2] ==> [2,2,2,2,2,2,2,2,2]
    	Object intArray[] = theirTestSet.getVector("[3*3*2]", "java.lang.Integer");
    	assertEquals("incorrect short array count", 9, intArray.length);
    	for (int i=0; i<intArray.length; i++) {
    		int val = (Integer) intArray[i];
    		assertEquals("strArray[" + i + "] should be 2", 2, val);
    	}    	
    	
    	
    	// [3*'2*3'] ==> ['2*3','2*3','2*3']
    	Object strArray[] = theirTestSet.getVector("[3*'2*3']", "java.lang.String");
    	assertEquals("incorrect array count", 3, strArray.length);
    	assertEquals("array[0]", "2*3", strArray[0]);
    	assertEquals("array[1]", "2*3", strArray[1]);
    	assertEquals("array[2]", "2*3", strArray[2]);
   	
    	// [3*ab] ==> [ab,ab,ab]
    	strArray = theirTestSet.getVector("[3*ab]", "java.lang.String");
    	assertEquals("incorrect str array count", 3, strArray.length);
    	for (int i=0; i<strArray.length; i++) {
    		String s = (String) strArray[1];
    		assertEquals("strArray[" + i + "] should be ab", "ab", s);
    	}    	
   	
    	
    	// [2*3*ab] ==> [ab,ab,ab,ab,ab,ab]
    	strArray = theirTestSet.getVector("[2*3*ab]", "java.lang.String");
    	assertEquals("incorrect str array count", 6, strArray.length);
    	for (int i=0; i<strArray.length; i++) {
    		String s = (String) strArray[1];
    		assertEquals("strArray[" + i + "] should be ab", "ab", s);
    	}    	
    	
    	// [3*10,5*2] ==> [10,10,10,2,2,2,2,2]
    	intArray = theirTestSet.getVector("[3*10,5*2]", "java.lang.Integer");
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
    	intArray = theirTestSet.getVector("[3*(1,2,3,4)]", "java.lang.Integer");
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
    	intArray = theirTestSet.getVector("[3 * 1 .. 4]", "java.lang.Integer");
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
    	strArray = theirTestSet.getVector("[3*'10.5*ab']", "java.lang.String");
    	assertEquals("incorrect array count", 3, strArray.length);
    	assertEquals("array[0]", "10.5*ab", strArray[0]);
    	assertEquals("array[1]", "10.5*ab", strArray[1]);
    	assertEquals("array[2]", "10.5*ab", strArray[2]);

    	// [10.5*'ab'] ==> [10.5*'ab']
    	strArray = theirTestSet.getVector("[10.5*'ab']", "java.lang.String");
    	assertEquals("incorrect array count", 1, strArray.length);
    	assertEquals("array[0]", "10.5*'ab'", strArray[0]);
    	
    	// [3*10.5*'ab'] ==> [10.5*'ab',10.5*'ab',10.5*'ab']
    	strArray = theirTestSet.getVector("[3*10.5*'ab']", "java.lang.String");
    	assertEquals("incorrect array count", 3, strArray.length);
    	assertEquals("array[0]", "10.5*'ab'", strArray[0]);
    	assertEquals("array[1]", "10.5*'ab'", strArray[1]);
    	assertEquals("array[2]", "10.5*'ab'", strArray[2]);
    	
    	// [3*'ab'*2] ==> ['ab'*2,'ab'*2,'ab'*2]
    	strArray = theirTestSet.getVector("[3*'ab'*2]", "java.lang.String");
    	assertEquals("incorrect array count", 3, strArray.length);
    	assertEquals("array[0]", "'ab'*2", strArray[0]);
    	assertEquals("array[1]", "'ab'*2", strArray[1]);
    	assertEquals("array[2]", "'ab'*2", strArray[2]);
    	
    	// [3*ab*2] ==> [ab*2,ab*2,ab*2]
    	strArray = theirTestSet.getVector("[3*ab*2]", "java.lang.String");
    	assertEquals("incorrect array count", 3, strArray.length);
    	assertEquals("array[0]", "ab*2", strArray[0]);
    	assertEquals("array[1]", "ab*2", strArray[1]);
    	assertEquals("array[2]", "ab*2", strArray[2]);
    	
    	// [1*(1,2,3)] ==> [1,2,3]
    	intArray = theirTestSet.getVector("[1*(1,2,3)]", "java.lang.Integer");
    	assertEquals("incorrect array count", 3, intArray.length);
    	assertEquals("array[0]", 1, intArray[0]);
    	assertEquals("array[1]", 2, intArray[1]);
    	assertEquals("array[2]", 3, intArray[2]);
    	
    	// [(1,2,3)] ==> [(1,2,3)]
    	strArray = theirTestSet.getVector("[(1,2,3)]", "java.lang.String");
    	assertEquals("incorrect array count", 3, strArray.length);
    	assertEquals("array[0]", "(1", strArray[0]);
    	assertEquals("array[1]", "2", strArray[1]);
    	assertEquals("array[2]", "3)", strArray[2]);
    	
    	
    	// [ab013..010] ==> [ab013,ab012,ab011,ab010]
    	strArray = theirTestSet.getVector("[ab013..010]", "java.lang.String");
    	assertEquals("incorrect array count", 4, strArray.length);
    	assertEquals("array[0]", "ab013", strArray[0]);
    	assertEquals("array[1]", "ab012", strArray[1]);
    	assertEquals("array[2]", "ab011", strArray[2]);
    	assertEquals("array[3]", "ab010", strArray[3]);
    	
    	
    	
    	// [/aa000..2] ==> [/aa000,/aa001,/aa002]
    	strArray = theirTestSet.getVector("[/aa000..2]", "java.lang.String");
    	assertEquals("incorrect array count", 3, strArray.length);
    	assertEquals("array[0]", "/aa000", strArray[0]);
    	assertEquals("array[1]", "/aa001", strArray[1]);
    	assertEquals("array[2]", "/aa002", strArray[2]);

    	// [/aa000../aa2] ==> [/aa000,/aa001,/aa002]
    	strArray = theirTestSet.getVector("[/aa000../aa2]", "java.lang.String");
    	assertEquals("incorrect array count", 3, strArray.length);
    	assertEquals("array[0]", "/aa000", strArray[0]);
    	assertEquals("array[1]", "/aa001", strArray[1]);
    	assertEquals("array[2]", "/aa002", strArray[2]);
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
