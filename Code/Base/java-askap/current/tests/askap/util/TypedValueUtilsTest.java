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
import askap.util.TypedValueUtils;
import askap.interfaces.*;
import askap.util.Complex;

/**
 * Test class for TypedValueUtils.
 * 
 * @author David Brodrick
 */
public class TypedValueUtilsTest {
    private static final float FLOAT_DELTA = 1.0e-15f;

    @Test
    public void test_null() {
        TypedValue tv = TypedValueUtils.object2TypedValue(null);
        assertTrue(tv.type == TypedValueType.TypeNull);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertNull(o);
    }

    @Test
    public void test_float() {
        float val = 3.141f;
        TypedValue tv = TypedValueUtils.object2TypedValue(new Float(val));
        assertTrue(tv.type == TypedValueType.TypeFloat);
        assertEquals(val, ((TypedValueFloat) tv).value, FLOAT_DELTA);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof Float);
        assertEquals(val, ((Float) o).floatValue(), FLOAT_DELTA);
    }

    @Test
    public void test_double() {
        double val = 3.141d;
        TypedValue tv = TypedValueUtils.object2TypedValue(new Double(val));
        assertTrue(tv.type == TypedValueType.TypeDouble);
        assertEquals(val, ((TypedValueDouble) tv).value, FLOAT_DELTA);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof Double);
        assertEquals(val, ((Double) o).doubleValue(), FLOAT_DELTA);
    }

    @Test
    public void test_int() {
        int val = 20090721;
        TypedValue tv = TypedValueUtils.object2TypedValue(new Integer(val));
        assertTrue(tv.type == TypedValueType.TypeInt);
        assertEquals(val, ((TypedValueInt) tv).value);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof Integer);
        assertEquals(val, ((Integer) o).intValue());
    }

    @Test
    public void test_long() {
        long val = 20090721l;
        TypedValue tv = TypedValueUtils.object2TypedValue(new Long(val));
        assertTrue(tv.type == TypedValueType.TypeLong);
        assertEquals(val, ((TypedValueLong) tv).value);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof Long);
        assertEquals(val, ((Long) o).longValue());
    }

    @Test
    public void test_string() {
        String val = "let the test begin";
        TypedValue tv = TypedValueUtils.object2TypedValue(val);
        assertTrue(tv.type == TypedValueType.TypeString);
        assertEquals(val, ((TypedValueString) tv).value);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof String);
        assertEquals(val, ((String) o));
    }

    @Test
    public void test_boolean() {
        boolean val = true;
        TypedValue tv = TypedValueUtils.object2TypedValue(val);
        assertTrue(tv.type == TypedValueType.TypeBool);
        assertEquals(val, ((TypedValueBool) tv).value);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof Boolean);
        assertEquals(val, ((Boolean) o));
    }

    @Test
    public void test_complex() {
        double real = 3.141d;
        double imag = 2.718d;
        TypedValue tv = TypedValueUtils.object2TypedValue(Complex.factory(real, imag));
        assertTrue(tv.type == TypedValueType.TypeDoubleComplex);
        assertEquals(real, ((TypedValueDoubleComplex) tv).value.real, FLOAT_DELTA);
        assertEquals(imag, ((TypedValueDoubleComplex) tv).value.imag, FLOAT_DELTA);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof Complex);
        assertEquals(real, ((Complex) o).getReal(), FLOAT_DELTA);
        assertEquals(imag, ((Complex) o).getImag(), FLOAT_DELTA);
    }   
   /*
    @Test
    public void test_azel_direction() {
        double c1 = 3.141d;
        double c2 = 2.718d;
        AzElApp d = new AzElApp(c1, c2);
        TypedValue tv = TypedValueUtils.object2TypedValue(d);
        assertTrue(((TypedValueDirection)tv).value.sys==CoordSys.AZEL);
        assertTrue(((TypedValueDirection)tv).value.coord1==c1);
        assertTrue(((TypedValueDirection)tv).value.coord2==c2);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof AzElApp);
        assertTrue(((AzElApp)o).getAz()==c1);
        assertTrue(((AzElApp)o).getEl()==c2);
    }

    @Test
    public void test_j2000_direction() {
        double c1 = 3.141d;
        double c2 = 2.718d;
        J2000Mean d = new J2000Mean(c1, c2);
        TypedValue tv = TypedValueUtils.object2TypedValue(d);
        assertTrue(((TypedValueDirection)tv).value.sys==CoordSys.J2000);
        assertTrue(((TypedValueDirection)tv).value.coord1==c1);
        assertTrue(((TypedValueDirection)tv).value.coord2==c2);
        Object o = TypedValueUtils.typedValue2Object(tv);
        assertTrue(o instanceof J2000Mean);
        assertTrue(((J2000Mean)o).getRA()==c1);
        assertTrue(((J2000Mean)o).getDec()==c2);
    }
    */
}
