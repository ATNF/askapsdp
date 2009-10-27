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

import java.util.Properties;
import java.util.Map;
import java.io.*;

/**
 * Java representation of a ParameterSet.
 * 
 * @author David Brodrick, some code derived from ATOMS.java by David Loone.
 */
public class ParameterSet extends Properties {

  /** Create an empty ParameterSet. */
  public ParameterSet() {
    super();
  }

  /** Create a ParameterSet containing all of the entries from the existing Map. */
  public ParameterSet(Map<String, String> map) {
    super();
    putAll(map);
  }

  /** Create a ParameterSet containing populated by reading entries from the InputStream. */
  public ParameterSet(InputStream is) throws IOException {
    super();
    load(is);
  }

  /** Create a ParameterSet containing populated by reading entries from named file. */
  public ParameterSet(String filename) throws IOException {
    super();
    InputStream is = new FileInputStream(filename);
    load(is);
  }

  /**
   * Get a parameter as a boolean.
   * 
   * @param key The name of the parameter to get.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a boolean.
   */
  public boolean getBoolean(String key) throws NumberFormatException {
    try {
      String strval = getProperty(key);
      if (strval == null) {
        throw new NumberFormatException("Error parsing key \"" + key + "\" as boolean: key does not exist");
      }
      return (new Boolean(strval)).booleanValue();
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as boolean: " + e.getMessage());
    }
  }

  /**
   * Get a parameter as a boolean.
   * 
   * @param key The name of the parameter to get.
   * 
   * @param default The value to return if the key could not be found.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a boolean.
   */
  public boolean getBoolean(String key, boolean defaultValue) throws NumberFormatException {
    // The value to return.
    boolean result;

    try {
      // The parameter value.
      String value = getProperty(key);

      if (value == null) {
        result = defaultValue;
      } else {
        result = (new Boolean(getProperty(key))).booleanValue();
      }
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as boolean: " + e.getMessage());
    }

    return result;
  }

  /**
   * Get a parameter as an integer.
   * 
   * @param key The name of the parameter to get.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as an integer.
   */
  public int getInteger(String key) throws NumberFormatException {
    try {
      return (new Integer(getProperty(key))).intValue();
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as integer: " + e.getMessage());
    } catch (NullPointerException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as integer: key does not exist");
    }
  }

  /**
   * Get a parameter as an integer.
   * 
   * @param key The name of the parameter to get.
   * 
   * @param default The value to return if the key could not be found.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a boolean.
   */
  public int getInteger(String key, int defaultValue) throws NumberFormatException {
    // The value to return.
    int result;

    try {
      // The property value.
      String value = getProperty(key);

      if (value == null) {
        result = defaultValue;
      } else {
        result = (new Integer(getProperty(key))).intValue();
      }
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as integer: " + e.getMessage());
    }

    return result;
  }

  /**
   * Get a parameter as a long.
   * 
   * @param key The name of the parameter to get.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a long.
   */
  public long getLong(String key) throws NumberFormatException {
    try {
      return (new Long(getProperty(key))).longValue();
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as long: " + e.getMessage());
    } catch (NullPointerException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as long: key does not exist");
    }
  }

  /**
   * Get a parameter as a long.
   * 
   * @param key The name of the parameter to get.
   * 
   * @param default The value to return if the key could not be found.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a boolean.
   */
  public long getLong(String key, long defaultValue) throws NumberFormatException {
    // The value to return.
    long result;

    try {
      // The property value.
      String value = getProperty(key);

      if (value == null) {
        result = defaultValue;
      } else {
        result = (new Long(getProperty(key))).longValue();
      }
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as long: " + e.getMessage());
    }

    return result;
  }

  /**
   * Get a parameter as a float.
   * 
   * @param key The name of the parameter to get.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a float.
   */
  public float getFloat(String key) throws NumberFormatException {
    try {
      return (new Float(getProperty(key))).floatValue();
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as float: " + e.getMessage());
    } catch (NullPointerException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as float: key does not exist");
    }
  }

  /**
   * Get a parameter as a float.
   * 
   * @param key The name of the parameter to get.
   * 
   * @param default The value to return if the key could not be found.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a boolean.
   */
  public float getFloat(String key, float defaultValue) throws NumberFormatException {
    // The value to return.
    float result;

    try {
      // The property value.
      String value = getProperty(key);

      if (value == null) {
        result = defaultValue;
      } else {
        result = (new Float(getProperty(key))).floatValue();
      }
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as float: " + e.getMessage());
    }

    return result;
  }

  /**
   * Get a parameter as a double.
   * 
   * @param key The name of the parameter to get.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a double.
   */
  public double getDouble(String key) throws NumberFormatException {
    try {
      return (new Double(getProperty(key))).doubleValue();
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as double: " + e.getMessage());
    } catch (NullPointerException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as double: key does not exist");
    }
  }

  /**
   * Get a parameter as a double.
   * 
   * @param key The name of the parameter to get.
   * 
   * @param default The value to return if the key could not be found.
   * 
   * @return The value of the parameter.
   * 
   * @exception NumberFormatException Thrown when the value of the parameter could not be
   * parsed as a boolean.
   */
  public double getDouble(String key, double defaultValue) throws NumberFormatException {
    // The value to return.
    double result;

    try {
      // The property value.
      String value = getProperty(key);

      if (value == null) {
        result = defaultValue;
      } else {
        result = (new Double(getProperty(key))).doubleValue();
      }
    } catch (NumberFormatException e) {
      throw new NumberFormatException("Error parsing key \"" + key + "\" as double: " + e.getMessage());
    }

    return result;
  }

  /**
   * Get a parameter as a string.
   * 
   * @param key The name of the parameter to get.
   * 
   * @return The value of the parameter.
   */
  public String getString(String key) {
    return getProperty(key);
  }

  /**
   * Get a parameter as a string.
   * 
   * @param key The name of the parameter to get.
   * 
   * @param default The value to return if the key could not be found.
   * 
   * @return The value of the parameter.
   */
  public String getString(String key, String defaultValue) {
    // The property value.
    String value = getProperty(key);

    if (value == null) {
      value = defaultValue;
    }

    return value;
  }
};
