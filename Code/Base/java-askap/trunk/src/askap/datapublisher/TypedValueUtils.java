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

package askap.datapublisher;

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import askap.interfaces.*;
import askap.util.Complex;

/**
 * Some utility methods for converting between Ice TypedValue types and native Java types.
 * 
 * @author David Brodrick
 */
public class TypedValueUtils {
  /**
   * Convert a TypedValue to its equivalent Java object.
   * 
   * @param o The TypedValue to be converted.
   * @return The Java Object equivalent.
   * @throws IllegalArgumentException Thrown if the argument type cannot be converted.
   */
  public static Object typedValue2Object(TypedValue tv) throws IllegalArgumentException {
    if (tv == null || tv.type == TypedValueType.TypeNull) {
      return null;
    } else if (tv.type == TypedValueType.TypeFloat) {
      return new Float(((TypedValueFloat) tv).value);
    } else if (tv.type == TypedValueType.TypeDouble) {
      return new Double(((TypedValueDouble) tv).value);
    } else if (tv.type == TypedValueType.TypeInt) {
      return new Integer(((TypedValueInt) tv).value);
    } else if (tv.type == TypedValueType.TypeLong) {
      return new Long(((TypedValueLong) tv).value);
    } else if (tv.type == TypedValueType.TypeString) {
      return new String(((TypedValueString) tv).value);
    } else if (tv.type == TypedValueType.TypeBoolean) {
      return new Boolean(((TypedValueBoolean) tv).value);
    } else if (tv.type == TypedValueType.TypeComplex) {
      TypedValueComplex c = (TypedValueComplex)tv;
      return Complex.factory(c.real, c.imag);      
    } else {
      throw new IllegalArgumentException("TypedValueUtils.typedValue2Object: Unhandled data type " + tv.type);
    }
  }

  /**
   * Convert a Java object to its equivalent TypedValue type.
   * 
   * @param o The object to be converted.
   * @return The TypedValue equivalent.
   * @throws IllegalArgumentException Thrown if the argument cannot be converted to a
   * TypedValue.
   */
  public static TypedValue object2TypedValue(Object o) throws IllegalArgumentException {
    if (o == null) {
      return new TypedValue(TypedValueType.TypeNull);
    } else if (o instanceof Float) {
      return new TypedValueFloat(TypedValueType.TypeFloat, ((Float) o).floatValue());
    } else if (o instanceof Float) {
      return new TypedValueFloat(TypedValueType.TypeFloat, ((Float) o).floatValue());
    } else if (o instanceof Double) {
      return new TypedValueDouble(TypedValueType.TypeDouble, ((Double) o).doubleValue());
    } else if (o instanceof Integer) {
      return new TypedValueInt(TypedValueType.TypeInt, ((Integer) o).intValue());
    } else if (o instanceof Long) {
      return new TypedValueLong(TypedValueType.TypeLong, ((Long) o).longValue());
    } else if (o instanceof String) {
      return new TypedValueString(TypedValueType.TypeString, (String) o);
    } else if (o instanceof Boolean) {
      return new TypedValueBoolean(TypedValueType.TypeBoolean, ((Boolean) o).booleanValue());
    } else if (o instanceof Complex) {
      return new TypedValueComplex(TypedValueType.TypeComplex, ((Complex) o).getReal(), ((Complex) o).getImag());      
    } else {
      throw new IllegalArgumentException("TypedValueUtils.object2TypedValue: Unhandled data type " + o.getClass());
    }
  }

  /**
   * Convert a TypedValue Map to a Map containing the equivalent native Java objects.
   * @param in The Map to be converted.
   * @return A map containing the same keys and native object values.
   * @throws IllegalArgumentException Thrown when a TypedValue value cannot be converted to a Java equivalent.
   */
  public static Map<String, Object> typedValueMap2ObjectMap(Map<String, TypedValue> in) throws IllegalArgumentException {
    HashMap<String, Object> res = new HashMap<String, Object>(in.size());
    Iterator<String> i = in.keySet().iterator();
    while (i.hasNext()) {
      String thiskey = i.next();
      res.put(thiskey, typedValue2Object(in.get(thiskey)));
    }
    return res;
  }

  /**
   * Convert a Map to a Map of TypedValues.
   * @param in The Map to be converted.
   * @return A map containing the same keys and TypedValue values.
   * @throws IllegalArgumentException Thrown when a value cannot be converted to a TypedValue.
   */
  public static Map<String, TypedValue> objectMap2TypedValueMap(Map<String, Object> in) throws IllegalArgumentException {
    HashMap<String, TypedValue> res = new HashMap<String, TypedValue>(in.size());
    Iterator<String> i = in.keySet().iterator();
    while (i.hasNext()) {
      String thiskey = i.next();
      res.put(thiskey, object2TypedValue(in.get(thiskey)));
    }
    return res;
  }
}
