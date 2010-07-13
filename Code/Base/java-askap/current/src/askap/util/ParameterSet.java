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

import sun.security.krb5.internal.PAData;

import com.sun.org.apache.xml.internal.utils.IntVector;

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

	/**
	 * Create a ParameterSet containing all of the entries from the existing
	 * Map.
	 */
	public ParameterSet(Map<String, String> map) {
		super();
		putAll(map);
	}

	/**
	 * Create a ParameterSet containing populated by reading entries from the
	 * InputStream.
	 */
	public ParameterSet(InputStream is) throws IOException {
		super();
		load(is);
	}

	/**
	 * Create a ParameterSet containing populated by reading entries from named
	 * file.
	 */
	public ParameterSet(String filename) throws IOException {
		super();
		InputStream is = new FileInputStream(filename);
		load(is);
	}

	/**
	 * Get a parameter as a boolean.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a boolean.
	 */
	public boolean getBoolean(String key) throws NumberFormatException {
		try {
			String strval = getProperty(key);
			if (strval == null) {
				throw new NumberFormatException("Error parsing key \"" + key
						+ "\" as boolean: key does not exist");
			}
			return (new Boolean(strval)).booleanValue();
		} catch (NumberFormatException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as boolean: " + e.getMessage());
		}
	}

	/**
	 * Get a parameter as a boolean.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @param default The value to return if the key could not be found.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a boolean.
	 */
	public boolean getBoolean(String key, boolean defaultValue)
			throws NumberFormatException {
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
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as boolean: " + e.getMessage());
		}

		return result;
	}

	/**
	 * Get a parameter as an integer.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as an integer.
	 */
	public int getInteger(String key) throws NumberFormatException {
		try {
			return (new Integer(getProperty(key))).intValue();
		} catch (NumberFormatException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as integer: " + e.getMessage());
		} catch (NullPointerException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as integer: key does not exist");
		}
	}

	/**
	 * Get a parameter as an integer.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @param default The value to return if the key could not be found.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a boolean.
	 */
	public int getInteger(String key, int defaultValue)
			throws NumberFormatException {
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
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as integer: " + e.getMessage());
		}

		return result;
	}

	/**
	 * Get a parameter as a long.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a long.
	 */
	public long getLong(String key) throws NumberFormatException {
		try {
			return (new Long(getProperty(key))).longValue();
		} catch (NumberFormatException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as long: " + e.getMessage());
		} catch (NullPointerException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as long: key does not exist");
		}
	}

	/**
	 * Get a parameter as a long.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @param default The value to return if the key could not be found.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a boolean.
	 */
	public long getLong(String key, long defaultValue)
			throws NumberFormatException {
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
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as long: " + e.getMessage());
		}

		return result;
	}

	/**
	 * Get a parameter as a float.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a float.
	 */
	public float getFloat(String key) throws NumberFormatException {
		try {
			return (new Float(getProperty(key))).floatValue();
		} catch (NumberFormatException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as float: " + e.getMessage());
		} catch (NullPointerException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as float: key does not exist");
		}
	}

	/**
	 * Get a parameter as a float.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @param default The value to return if the key could not be found.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a boolean.
	 */
	public float getFloat(String key, float defaultValue)
			throws NumberFormatException {
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
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as float: " + e.getMessage());
		}

		return result;
	}

	/**
	 * Get a parameter as a double.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a double.
	 */
	public double getDouble(String key) throws NumberFormatException {
		try {
			return (new Double(getProperty(key))).doubleValue();
		} catch (NumberFormatException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as double: " + e.getMessage());
		} catch (NullPointerException e) {
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as double: key does not exist");
		}
	}

	/**
	 * Get a parameter as a double.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @param default The value to return if the key could not be found.
	 * 
	 * @return The value of the parameter.
	 * 
	 * @exception NumberFormatException
	 *                Thrown when the value of the parameter could not be parsed
	 *                as a boolean.
	 */
	public double getDouble(String key, double defaultValue)
			throws NumberFormatException {
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
			throw new NumberFormatException("Error parsing key \"" + key
					+ "\" as double: " + e.getMessage());
		}

		return result;
	}

	/**
	 * Get a parameter as a string.
	 * 
	 * @param key
	 *            The name of the parameter to get.
	 * 
	 * @return The value of the parameter.
	 */
	public String getString(String key) {
		return getProperty(key);
	}

	/**
	 * Get a parameter as a string.
	 * 
	 * @param key
	 *            The name of the parameter to get.
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

	/**
	 * Create a new ParameterSet which only contains keys which start with the
	 * specified prefix. The prefix string will be removed from the key names in
	 * the returned set. The caller is expected to provide the delimiting "." if
	 * the delimiter is not desired in the key names of the returned subset.
	 * 
	 * @param prefix
	 *            The string that keys must start with.
	 * @return ParameterSet containing matching keys and values, with prefix
	 *         removed from key names. Will have a zero size if no keys matched.
	 */
	public ParameterSet subset(String prefix) {
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
	 * Given a key and type, convert the value for the key typed object
	 * 
	 * @param key
	 * @param type
	 *            : type has to be full class name, eg: java.lang.Integer
	 * @return
	 * @throws NumberFormatException
	 */
	public Object getObject(String key, String type)
			throws NumberFormatException {
		String value = getProperty(key);
		return getTypedValue(value, type);
	}

	/**
	 * Given a string value and type, convert the string value to a typed object
	 * 
	 * @param value
	 * @param type
	 *            : type has to be full class name, eg: java.lang.Integer
	 * @return
	 * @throws NumberFormatException
	 */
	public static Object getTypedValue(String value, String type)
			throws NumberFormatException {
		if (value == null || value.length() == 0)
			return null;

		return getSimpleObject(value, type);
	}

	/**
	 * vector value should begin and end with [], values are delimited by ","
	 * white spaces at the beginning and end of each value are trimmed
	 * 
	 * @param key
	 * @param type
	 *            is the type of each element in the array. eg: if type is
	 *            java.lang.Long, then return an array of Long
	 * @return an array of Objects
	 * @throws NumberFormatException
	 */
	public Object[] getVectorValue(String key, String type)
			throws NumberFormatException {
		String value = getProperty(key);
		if (value == null || value.length() == 0)
			return null;

		return getVector(value, type);
	}

	/**
	 * vector value should begin and end with [], values are delimited by ","
	 * white spaces at the beginning and end of each value are trimmed
	 * 
	 * @param value
	 * @param type
	 *            is the type of each element in the array. eg: if type is
	 *            java.lang.Long, then return an array of Long
	 * @return an array of Objects
	 * @throws NumberFormatException
	 */
	public static Object[] getVector(String value, String type)
			throws NumberFormatException {

		if (value == null || value.length() < 2 || !value.startsWith("[")
				|| !value.endsWith("]"))
			throw new NumberFormatException("Value " + value
					+ " is null or in wrong formate for vector");
		
		String str = expandMultString(value);
		str = expandRangeString(str);
		str = str.substring(1, str.length()-1);

		if (str.length() == 0)
			return new Object[0];

		String strValues[] = str.split(",");
		Object objValues[] = new Object[strValues.length];

		for (int i = 0; i < strValues.length; i++) {
			objValues[i] = getSimpleObject(strValues[i].trim(), type);
		}

		return objValues;
	}

	private static Object getSimpleObject(String value, String type)
			throws NumberFormatException {
		Object o = null;
		if (type == null || type.length() == 0)
			return value;

		if (value == null || value.length() == 0)
			return null;

		Class<?> c;
		try {
			c = Class.forName(type);
			Class<String> strArgsClass = String.class;
			Constructor<?> constructor = c.getConstructor(strArgsClass);
			o = constructor.newInstance(value);
		} catch (Exception e) {
			String msg = "Could not convert " + value + " to " + type;
			if (e.getMessage() != null)
				msg = msg + ": " + e.getMessage();
			throw new NumberFormatException(msg);
		}

		return o;
	}

	/**
	 * print object out in proper ParameterSet format For simple type simply get
	 * string value For vectors, format is eg: [val1, vale2]
	 * 
	 * @param o
	 * @return
	 */
	public static String getStrValue(Object o) {
		if (o == null)
			return null;

		if (o instanceof Object[]) {
			Object[] list = (Object[]) o;
			String val = "[";
			for (Object x : list) {
				val = val + x.toString() + ",";
			}
			// get rid of last ,
			if (val.length() > 1)
				val = val.substring(0, val.length() - 1);
			val = val + "]";

			return val;

		} else {
			return o.toString();
		}
	}

	//
	// ****************************************************************************************
	// ALL THE FOLLOWING CODE ARE PRETTY MUCH COPIED FROM LOFAR Common-3.3
	// StringUtil.cc
	// ****************************************************************************************
	//

	private static int skipBalanced(String str, int st, int end, char endChar)
			throws NumberFormatException {
		char ch = str.charAt(st++);
		int nrp = 1;
		while (st < end) {
			// First test on end character. In this way it also works well
			// if start and end character are the same.
			if (str.charAt(st) == endChar) {
				st++;
				if (--nrp == 0)
					return st;
			} else if (str.charAt(st) == ch) {
				st++;
				nrp++;
			} else if (str.charAt(st) == '"' || str.charAt(st) == '\'') {
				st = skipQuoted(str, st);
			} else {
				st++;
			}
		}
		throw new NumberFormatException("Unbalanced " + ch + endChar + " in "
				+ str);
	}

	private static boolean isdigit(char charAt) {

		if (charAt == '0' || charAt == '1' || charAt == '2' || charAt == '3'
				|| charAt == '4' || charAt == '5' || charAt == '6'
				|| charAt == '7' || charAt == '8' || charAt == '9')
			return true;

		return false;
	}

	private static int skipQuoted(String str, int st) throws NumberFormatException {
		int pos = str.indexOf(str.charAt(st), st + 1);
		if (pos == -1)
			throw new NumberFormatException(
					"Unbalanced quoted string at position " + st + " in " + str);
		return pos + 1;
	}

	/*
	 * This function replaces the string between (start, start+numberOfChar) of
	 * origStr with replaceStr
	 */
	private static String replace(String origStr, int start, int numberOfChar,
			String replaceStr) {
		String lstr = origStr.substring(0, start);
		String rstr = origStr.substring(start + numberOfChar);

		return (lstr + replaceStr + rstr);
	}

	private static int rskipws(String value, int st, int end) {
		for (; end > st; --end) {
			if (value.charAt(end - 1) != ' ' && value.charAt(end - 1) != '\t') {
				break;
			}
		}
		return end;
	}

	private static int lskipws(String value, int st, int end) {
		for (; st < end; ++st) {
			if (value.charAt(st) != ' ' && value.charAt(st) != '\t') {
				break;
			}
		}
		return st;
	}

	/**
	 * pad the given number to a string with given length with given padding on
	 * the left
	 */
	private static  String getPaddedString(int number, char padding, int length) {
		String str = "" + number;

		while (str.length() < length)
			str = padding + str;

		return str;
	}

	//
	// expandedArrayString(string)
	//
	// Given een array string ( '[ xx..xx, xx ]' ) this utility expands the
	// string
	// by replacing ranges with the fill series.
	// Eg. [ lii001..lii003, lii005 ] --> [ lii001, lii002, lii003, lii005 ]
	// [ 10*0 ] --> [ 0,0,0,0,0,0,0,0,0,0 ]
	// [ 3*(0;1;2;3) ] --> [ 0,1,2,3,0,1,2,3,0,1,2,3 ]
	// [ 3*(300..303) ] --> [ 300,301,302,303,300,301,302,303,300,301,302,303 ]
	// [ 2*(5*0) ] --> [ 0,0,0,0,0,0,0,0,0,0 ]

	private static String expandRangeString(String strng) throws NumberFormatException {
		String str = strng;
		int i = 0;
		int last = str.length();
		while (i < last - 1) {
			if (str.charAt(i) == '\'' || str.charAt(i) == '"') {
				// Ignore a quoted part.
				int pos = str.indexOf(str.charAt(i), i + 1);

				if (pos == -1) {
					throw new NumberFormatException(
							"Unbalanced quoted string at position " + i
									+ " in " + str);
				}
				i = pos;

			} else if (str.charAt(i) == '.' && str.charAt(i + 1) == '.') {
				// Found ..; look back for number and prefix.
				// First find number.
				int endnum = rskipws(str, 0, i);
				int stnum = endnum - 1;
				while (stnum >= 0 && isdigit(str.charAt(stnum))) {
					--stnum;
				}
				int lennum = endnum - stnum - 1;
				if (lennum > 0) {
					int num = ((Integer) getTypedValue(str.substring(stnum+1,
							endnum), "java.lang.Integer")).intValue();
					// Found number, now find possible prefix.
					// We could say that the prefix has to be alphanumeric, but
					// more
					// general is to accept all characters except ([,*; blank
					// and tab.
					int stalp = stnum;
					while (stalp >= 0 && str.charAt(stalp) != '('
							&& str.charAt(stalp) != '['
							&& str.charAt(stalp) != ','
							&& str.charAt(stalp) != ';'
							&& str.charAt(stalp) != '*'
							&& str.charAt(stalp) != ' '
							&& str.charAt(stalp) != '\t') {
						--stalp;
					}
					stalp++;
					String prefix = str.substring(stalp, stnum + 1);
					String suffix = "";
					// Now find part after the .. which can contain the same
					// prefix.
					// if no parenthesis was used.
					i = lskipws(str, i + 2, last);
					if (prefix.length() > 0 && last - i > prefix.length()
							&& str.substring(i, i + prefix.length()).equals(prefix)) {
						i += prefix.length();
					}
					// Test if a digit.
					if (isdigit(str.charAt(i))) {
						stnum = i;
						// Skip to end of number.
						while (i < last && isdigit(str.charAt(i))) {
							++i;
						}
						endnum = ((Integer) getTypedValue(str.substring(stnum,
								i), "java.lang.Integer")).intValue();
						// We really have something like xxx000..004
						// Find a possible suffix.
						int stsuf = i;
						while (i < last && str.charAt(i) != ')'
								&& str.charAt(i) != ']' && str.charAt(i) != ','
								&& str.charAt(i) != ';' && str.charAt(i) != '*'
								&& str.charAt(i) != ' '
								&& str.charAt(i) != '\t') {
							++i;
						}
						if (i > stsuf) {
							suffix = str.substring(stsuf, i);
						}
						// Removes braces if the prefix ends and suffix starts
						// with it.
						int lpre = prefix.length();
						int lsuf = suffix.length();
						if (lpre > 0 && lsuf > 0
								&& prefix.charAt(lpre - 1) == '{'
								&& suffix.charAt(0) == '}') {
							prefix = prefix.substring(0, lpre - 1);
							suffix = suffix.substring(1, lsuf);
						}
						// Fill it in in ascending or descending order.
						String ostr = "";
						if (num < endnum) {
							for (; num <= endnum; ++num) {
								ostr = ostr + prefix
										+ getPaddedString(num, '0', lennum)
										+ suffix;
								if (num != endnum)
									ostr += ',';
							}
						} else {
							for (; num >= endnum; --num) {
								ostr = ostr + prefix
										+ getPaddedString(num, '0', lennum)
										+ suffix;
								if (num != endnum)
									ostr += ',';
							}
						}
						str = replace(str, stalp, i - stalp, ostr);
						int diff = ostr.length() - (i - stalp);
						i += diff;
						last += diff;
					}
				}
			}
			++i;
		}
		return str;
	}

	private static String expandMultString(String strng) {
		String str = strng;
		int i = 0;
		int last = str.length();
		while (i < last - 1) {
			if (str.charAt(i) == '\'' || str.charAt(i) == '"') {
				// Ignore a quoted part.
				i = skipQuoted(str, i);
			} else {
				if (str.charAt(i) != '*') {
					++i;
				} else {
					// Found *; look back for digits.
					int endnum = rskipws(str, 0, i);
					int stnum = endnum - 1;
					while (stnum >= 0 && isdigit(str.charAt(stnum))) {
						--stnum;
					}
					stnum++;
					// Only use it if the number is at begin or preceeded by a
					// delimiter.
					int j = rskipws(str, 0, stnum);
					int lennum = 0;
					if (j == 0 || str.charAt(j - 1) == ','
							|| str.charAt(j - 1) == '['
							|| str.charAt(j - 1) == '(') {
						lennum = endnum - stnum;
					}
					if (lennum == 0) {
						// No number found, so ignore the *.
						++i;
					} else {
						boolean continueAtReplace = true;
						int num = ((Integer) getTypedValue(str.substring(stnum,
								stnum + lennum), "java.lang.Integer")).intValue();
						// Now find the string that has to be multiplied.
						// This can be a single instance or a set indicated by
						// () or [].
						// First skip possible whitespace after *.
						i = lskipws(str, i + 1, last);
						int stval = i;
						String val = "";
						if (str.charAt(i) == '[') {
							// A set in [].
							i = skipBalanced(str, i, last, ']');
							val = str.substring(stval, i);
						} else if (str.charAt(i) == '(') {
							// A set in (). Remove the parentheses.
							i = skipBalanced(str, i, last, ')');
							val = str.substring(stval + 1, i - 1);
							// Replace ; by , (for backward compatibility).
							int stv = 0;
							while (stv < val.length()) {
								if (val.charAt(stv) == '"'
										|| val.charAt(stv) == '\'') {
									stv = skipQuoted(val, stv);
								} else {
									if (val.charAt(stv) == ';') {
										val = replace(val, stv, 1, ",");
									}
									stv++;
								}
							}
						} else {
							// Any other value is ended by ,]).
							while (i < last) {
								if (str.charAt(i) == '"'
										|| str.charAt(i) == '\'') {
									// A quoted string.
									i = skipQuoted(str, i);
								} else if (str.charAt(i) == '[') {
									// Another string in brackets.
									i = skipBalanced(str, i, last, ']');
								} else if (str.charAt(i) == '(') {
									// Another string in parentheses.
									i = skipBalanced(str, i, last, ')');
								} else {
									if (str.charAt(i) == ','
											|| str.charAt(i) == ']'
											|| str.charAt(i) == ')') {
										// The end of the value.
										break;
									}
									// Continue with next character.
									++i;
								}
							}
							val = str.substring(stval, i);
							// / continueAtReplace = false;
						}
						// Insert the values num times separated by a comma.
						String res = "";
						for (j = 0; j < num; ++j) {
							if (j > 0)
								res += ',';
							res += val;
						}
						// Replace the value by the new result.
						str = replace(str, stnum, i - stnum, res);
						last += res.length() - (i - stnum);
						// Continue scanning at start of replace (for possible
						// recursion).
						if (continueAtReplace) {
							i = stnum;
						} else {
							i = stnum + res.length();
						}
					}
				}
			}
		}
		return str;
	}

};
