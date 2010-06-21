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

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Constructor;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

/**
 * Java representation of a ParameterSet.
 * 
 * @author David Brodrick, some code derived from ATOMS.java by David Loone.
 */
public class ParameterSet extends Properties
{

    /** Create an empty ParameterSet. */
    public ParameterSet()
    {
        super();
    }

    /** Create a ParameterSet containing all of the entries from the existing Map. */
    public ParameterSet(Map<String, String> map)
    {
        super();
        putAll(map);
    }

    /** Create a ParameterSet containing populated by reading entries from the InputStream. */
    public ParameterSet(InputStream is) throws IOException
    {
        super();
        load(is);
    }

    /** Create a ParameterSet containing populated by reading entries from named file. */
    public ParameterSet(String filename) throws IOException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a boolean.
     */
    public boolean getBoolean(String key) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a boolean.
     */
    public boolean getBoolean(String key, boolean defaultValue) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as an integer.
     */
    public int getInteger(String key) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a boolean.
     */
    public int getInteger(String key, int defaultValue) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a long.
     */
    public long getLong(String key) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a boolean.
     */
    public long getLong(String key, long defaultValue) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a float.
     */
    public float getFloat(String key) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a boolean.
     */
    public float getFloat(String key, float defaultValue) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a double.
     */
    public double getDouble(String key) throws NumberFormatException
    {
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
     * @exception NumberFormatException Thrown when the value of the parameter could not
     * be parsed as a boolean.
     */
    public double getDouble(String key, double defaultValue) throws NumberFormatException
    {
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
    public String getString(String key)
    {
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
    public String getString(String key, String defaultValue)
    {
        // The property value.
        String value = getProperty(key);

        if (value == null) {
            value = defaultValue;
        }

        return value;
    }

    /**
     * Create a new ParameterSet which only contains keys which start with the specified
     * prefix. The prefix string will be removed from the key names in the returned set.
     * The caller is expected to provide the delimiting "." if the delimiter is not
     * desired in the key names of the returned subset.
     * 
     * @param prefix The string that keys must start with.
     * @return ParameterSet containing matching keys and values, with prefix removed from
     * key names. Will have a zero size if no keys matched.
     */
    public ParameterSet subset(String prefix)
    {
        ParameterSet res = new ParameterSet();
        int prefixlen = prefix.length();
        Iterator<Object> i = keySet().iterator();
        while (i.hasNext()) {
            String thiskey = (String) i.next();
            if (thiskey.startsWith(prefix)) {
                // Key matches, remove prefix and insert into result
                res.put(thiskey.substring(prefixlen), getProperty(thiskey));
            }
        }
        return res;
    }
    

    /**
     * Get a parameter as an Object of a given type
     * 
     * @param key The name of the parameter to get.
     * @param the type of the object to be returned
     * @return The typed Object value of the parameter.
     */
	public Object getObject(String key, String type) throws NumberFormatException {
		String value = getProperty(key);
		return getTypedValue(value, type);
	}
	
	/**
	 * Given a string value and type, convert hte string value to a typed object
	 * 
	 * @param value
	 * @param type
	 * @return
	 * @throws NumberFormatException
	 */
	public static Object getTypedValue(String value, String type) throws NumberFormatException {
		if (value==null || value.length()==0)
			return null;
		
		if (value.startsWith("[") && value.endsWith("]"))
			return getVector(value, type);
		else
			return getSimpleObject(value, type);
	}
	
	
	/**
	 * value should begin and end with [], values are delimited by ","
	 * white spaces are trimmed
	 * 
	 * @param value
	 * @param type
	 * @return an array of Objects
	 * @throws NumberFormatException
	 */
	private static Object getVector(String value, String type)  throws NumberFormatException {
		
		if (value==null || value.length()<2 || !value.startsWith("[") || !value.endsWith("]"))
			throw new NumberFormatException("Value " + value + " is null or in wrong formate for vector");
		
		String str = value.substring(1, value.length()-1);
		
		if (str.length()==0)
			return new Object[0];
		
		String strValues[] = str.split(",");
		Object objValues[] = new Object[strValues.length];
		
		for (int i=0; i<strValues.length; i++) {
			objValues[i] = getSimpleObject(strValues[i].trim(), type);
		}
		
		return objValues;
	}

	private static Object getSimpleObject(String value, String type) throws NumberFormatException {
		Object o = null;
		if (type==null || type.length()==0 || type.equalsIgnoreCase("string"))
			return value;
		
		if (value==null || value.length()==0)
			return null;
		
		if (type.equalsIgnoreCase("short")
				|| type.equalsIgnoreCase("byte")
				|| type.equalsIgnoreCase("long")
				|| type.equalsIgnoreCase("boolean")
				|| type.equalsIgnoreCase("integer")
				|| type.equalsIgnoreCase("double")
				|| type.equalsIgnoreCase("float")){
			// handle simple types such Integer, Double etc
			Class<?> c;
			try {
				c = Class.forName("java.lang." + type);
				Class<String> strArgsClass = String.class;
				Constructor<?> constructor = c.getConstructor(strArgsClass);
				o = constructor.newInstance(value);
			} catch (Exception e) {
				String msg = "Could not convert " + value + " to " + type;
				if (e.getMessage() != null)
					msg = msg + ": " + e.getMessage();
				throw new NumberFormatException(msg);
			}
		} else {
			throw new NumberFormatException("Unknown type " + type);
		}
		
		return o;	
	}
	
	/**
	 * print object out in proper ParameterSet format
	 * @param o
	 * @return
	 */
	public static String getStrValue(Object o) {
		if (o==null)
			return null;
		
		if (o instanceof Object[]) {
			Object[] list = (Object[]) o;
			String val = "[";
			for (Object x : list) {
				val = val + x.toString() + ",";
			}
			// get rid of last ,
			if (val.length()>1)
			val = val.substring(0, val.length()-1);
			val = val + "]";
			
			return val;
			
		} else {
			return o.toString();
		}
		
	}
};
