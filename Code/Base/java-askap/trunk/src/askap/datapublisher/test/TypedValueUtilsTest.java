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

package askap.datapublisher.test;

import org.junit.*;
import static org.junit.Assert.*;
import askap.datapublisher.TypedValueUtils;
import askap.interfaces.*;
import askap.util.Complex;

/**
 * Test class for TypedValueUtils.
 * 
 * @author David Brodrick
 */
public class TypedValueUtilsTest {
  @Test
  public void test_null() {
    TypedValue tv = TypedValueUtils.object2TypedValue(null);
    assertTrue(tv.type == TypedValueType.TypeNull);
    Object o = TypedValueUtils.typedValue2Object(tv);
    assertTrue(o == null);
  }

  @Test
  public void test_float() {
    float val = 3.141f;
    TypedValue tv = TypedValueUtils.object2TypedValue(new Float(val));
    assertTrue(tv.type == TypedValueType.TypeFloat);
    assertTrue(((TypedValueFloat) tv).value == val);
    Object o = TypedValueUtils.typedValue2Object(tv);
    assertTrue(o instanceof Float);
    assertTrue(((Float) o).floatValue() == val);
  }

  @Test
  public void test_double() {
    double val = 3.141d;
    TypedValue tv = TypedValueUtils.object2TypedValue(new Double(val));
    assertTrue(tv.type == TypedValueType.TypeDouble);
    assertTrue(((TypedValueDouble) tv).value == val);
    Object o = TypedValueUtils.typedValue2Object(tv);
    assertTrue(o instanceof Double);
    assertTrue(((Double) o).doubleValue() == val);
  }

  @Test
  public void test_int() {
    int val = 20090721;
    TypedValue tv = TypedValueUtils.object2TypedValue(new Integer(val));
    assertTrue(tv.type == TypedValueType.TypeInt);
    assertTrue(((TypedValueInt) tv).value == val);
    Object o = TypedValueUtils.typedValue2Object(tv);
    assertTrue(o instanceof Integer);
    assertTrue(((Integer) o).intValue() == val);
  }

  @Test
  public void test_long() {
    long val = 20090721l;
    TypedValue tv = TypedValueUtils.object2TypedValue(new Long(val));
    assertTrue(tv.type == TypedValueType.TypeLong);
    assertTrue(((TypedValueLong) tv).value == val);
    Object o = TypedValueUtils.typedValue2Object(tv);
    assertTrue(o instanceof Long);
    assertTrue(((Long) o).longValue() == val);
  }

  @Test
  public void test_string() {
    String val = "let the test begin";
    TypedValue tv = TypedValueUtils.object2TypedValue(val);
    assertTrue(tv.type == TypedValueType.TypeString);
    assertTrue(((TypedValueString) tv).value.equals(val));
    Object o = TypedValueUtils.typedValue2Object(tv);
    assertTrue(o instanceof String);
    assertTrue(((String) o).equals(val));
  }

  @Test
  public void test_boolean() {
    boolean val = true;
    TypedValue tv = TypedValueUtils.object2TypedValue(new Boolean(val));
    assertTrue(tv.type == TypedValueType.TypeBool);
    assertTrue(((TypedValueBool) tv).value == val);
    Object o = TypedValueUtils.typedValue2Object(tv);
    assertTrue(o instanceof Boolean);
    assertTrue(((Boolean) o).booleanValue() == val);
  }
  
  @Test
  public void test_complex() {
    double real = 3.141d;
    double imag = 2.718d;
    TypedValue tv = TypedValueUtils.object2TypedValue(Complex.factory(real, imag));
    assertTrue(tv.type == TypedValueType.TypeDoubleComplex);
    assertTrue(((TypedValueDoubleComplex) tv).value.real == real);
    assertTrue(((TypedValueDoubleComplex) tv).value.imag == imag);
    Object o = TypedValueUtils.typedValue2Object(tv);
    assertTrue(o instanceof Complex);
    assertTrue(((Complex) o).getReal() == real);
    assertTrue(((Complex) o).getImag() == imag);
  }  
}
