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
package askap.util;

import java.io.PrintStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

/**
 * A simple command line parser. The external behaviour of this is loosly based on 
 * the C++ Boost.Program_options package. 
 */
public class CmdLineParser {
	
	/**
	 * Internal structure used to represent an expected or optional command line option.
	 */
	private static class ProgramOption {
		String keyLong;
		String keyShort;
		String desc;
		boolean hasValue;
	}

	/**
	 * Stores a list of expected or optional command line options.
	 */
	private Vector<ProgramOption> itsOptions;

	/**
	 * Constructor
	 */
	public CmdLineParser() {
		itsOptions = new Vector<ProgramOption>();
		addOption("help", "h", "produce help message", false);
	}

	/**
	 * Adds an optional or expected (there is no difference here) program option.
	 * 
	 * @param keyLong		a long version of the key, e.g. "config" will result
	 * 						in the command line option "--config" matching.
	 * @param keyShort		a short version of the key, eg. "c" will result in the
	 * 						command line option "-c" matching.
	 * @param description	a description of the option. This will be used in the 
	 * 						help output.
	 * @param hasValue		true if the option is expected to be followed by a value,
	 * 						otherwise false.
	 */
	public void addOption(String keyLong, String keyShort, String description, boolean hasValue) {
		// Pre-conditions
		if (keyShort.length() != 1) {
			throw new RuntimeException("KeyShort must be a single character");
		}

		if (keyLong.length() < 2) {
			throw new RuntimeException("KeyLong must be at least two characters");
		}

		// First search through existing options searching for duplicate keys
		for (ProgramOption po : itsOptions) {
			if (po.keyLong.equalsIgnoreCase(keyLong)
					|| po.keyShort.equalsIgnoreCase(keyShort)) {
				throw new RuntimeException("Duplicate key: " + keyShort + "/" + keyLong);
			}
		}

		ProgramOption po = new ProgramOption();
		po.keyLong = keyLong;
		po.keyShort = keyShort;
		po.desc = description;
		po.hasValue = hasValue;
		itsOptions.add(po);
	}


	/**
	 * Parses command line arguments and returns a map of options (keys)
	 * and values, or an empty string in the case a value is not expected.
	 * 
	 * @param args	command line argument array
	 * @return		a map of all command line options and values.
	 */
	public Map<String, String> parse(String args[]) {		
		Map<String, String> options = new HashMap<String, String>();

		for (int i = 0; i < args.length; i++) {
			String s = args[i];
			ProgramOption po = null;
			if (s.startsWith("--")) {
				po = lookupOption(s.substring(2));
			} else if (s.startsWith("-")) {
				po = lookupOption(s.substring(1));
			} else {

			}

			if (po != null) {
				options.put(po.keyLong, parseOption(po, args, i));
				if (po.hasValue) i++;
			} else {
				throw new RuntimeException("Option \" " + s + "\" not a valid program option");
			}
		}

		if (args.length == 0 || options.containsKey("help")) {
			help(System.err);
		}

		return options;
	}

	/**
	 * Writes the command line usage (i.e help) to a PrintStream.
	 * @param stream the stream to which the help will be written.
	 */
	public void help(PrintStream stream) {
		stream.println("Program Options:");
		for (ProgramOption po : itsOptions) {
			stream.print("  " + "-" + po.keyShort + "[ --" + po.keyLong + " ]");
			if (po.hasValue) {
				stream.print(" arg");
			}
			stream.println('\t' + po.desc);
		}

		System.exit(1);
	}

	/**
	 * Helper function for parsing command line parameters. Once a key is
	 * found, either prefixed with a single or double hyphen, this method is
	 * called. Given a ProgramOption matching the key found, the cmdline
	 * argument array and the index of the matched key, this method will
	 * 1) return "" if the program option isn't expected to have a value, or
	 * 2) return the string for the value if is is expected to have a value, or
	 * 3) throws an exception in the case a value was expected but one was not
	 * provided.
	 * 
	 * @param po	the program option object that corresponds to the
	 * 				parameter at args[i]
	 * @param args	the command line args being parsed
	 * @param i		the index (zero-based) that the current option was found at
	 * @return  the value following this option (at i) or an empty string if
	 * 			there the program option has no value.
	 */
	private String parseOption(ProgramOption po, String[] args, int i) {
		if (po.hasValue) {
			if (i < args.length - 1) {
				return args[i + 1];
			} else {
				throw new RuntimeException("Option " + args[i] + " requires an argument");
			}
		} else {
			return "";
		}
	}

	/**
	 * Searches the ProgramOptions list associated with this object (options are
	 * added with the addOption() method) for a ProgramOption which has a key (either
	 * keyLong or keyShort) matching the "key" parameter.
	 * 
	 * @param key 	the key to search for
	 * @return		the ProgramOption instance matching key, or null if no match was
	 * 				found.
	 */
	private ProgramOption lookupOption(String key) {
		for (ProgramOption po : itsOptions) {
			if (po.keyLong.equalsIgnoreCase(key)
					|| po.keyShort.equalsIgnoreCase(key)) {
				return po;
			}
		}
		return null;
	}
}
