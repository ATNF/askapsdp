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
package askap.cp.manager;

import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Enumeration;
import java.util.Map;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.apache.log4j.MDC;
import org.apache.log4j.PropertyConfigurator;
import org.apache.log4j.xml.DOMConfigurator;




// Local package includes
import askap.util.ParameterSet;

/**
 * This class encapsulates the initialisation and shutdown of an Ice
 * Service. It provides the following functionality:
 * <ul>
 * <li> Standard approach to command line parameters
 * <li> Usage/help message
 * <li> Setup logging
 * <li> Parsing of ParameterSet configuration file
 * <li> Handling of exceptions so they don't propagate out of main()
 * </ul>
 * 
 * Here is an example of usage:
 * <pre>
 * public class CpManager extends ServiceApplication {
 *     @Override
 *     public int run(String[] args) {
 *         // Your code goes here
 *         
 *         // You can get the configuration parset like this:
 *         ParameterSet parset = config();
 *     }
 *     
 *     public static void main(String[] args) {
 *	       CpManager svr = new CpManager(SERVICE_NAME);
 *		   int status = svr.servicemain(args);
 *		   System.exit(status);
	   }
 * }
 * </pre>
 */
public abstract class ServiceApplication {

	/** Logger */
	private static Logger logger = Logger.getLogger(
			ServiceApplication.class.getName());

	/** Service name */
	String itsServiceName;

	/** Command line parser */
	CmdLineParser itsCmdlineParser = new CmdLineParser();

	/** Command line parameters */
	private Map<String, String> itsCmdlineParameters;

	/** Parameter Set from configuration file */
	private ParameterSet itsParset;

	/** Ice Communicator */
	Ice.Communicator itsCommunicator;

	public ServiceApplication(String serviceName) {
		itsServiceName = serviceName;
	}

	/**
	 * This function is implemented by sub-classes. i.e. The users of this class.
	 * 
	 * @param args	command line arguments
	 * @return 		exit status
	 */
	public abstract int run(String args[]);

	/**
	 * This must be called by the user, typically in the program main().
	 * It performs initialisation, calls the user implemented run() method,
	 * then performs any necessary finalisation.
	 * 
	 * @param args	command line arguments
	 * @return		exit status
	 */
	public int servicemain(String args[]) {
		int status = 1;

		try {
			processCmdLineArgs(args);
			if (!parameterExists("config")) {
				System.err.println("Error: Configuration file not specified");
				return 1;
			}

			initLogging(parameter("log-config"));
			initConfig(parameter("config"));
			initIce();

			// Run the application's run() method
			status = run(args);

			// Shutdown
			shutdownIce();
		} catch (Exception e) {
			if (Logger.getRootLogger().getAllAppenders().hasMoreElements()) {
				logger.fatal("Error: " + e.getMessage());
				e.printStackTrace();
			} else {
				System.err.println("Error: " + e.getMessage());
			}
			return 1;
		}

		return status;
	}

	/**
	 * Returns the configuration parameter set that was read in from
	 * the file specified via the "--config" or "-c" command line parameter.
	 * @return a paramter set.
	 */
	public ParameterSet config() {
		return itsParset;
	}

	/**
	 * Adds an optional or expected (there is no difference here) command line parameter.
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
	public void addParameter(String keyLong, String keyShort,
			String description, boolean hasValue) {
		if (itsCmdlineParameters != null) {
			throw new RuntimeException(
					"Must call addParameter() before parsing");
		}
		itsCmdlineParser.addOption(keyLong, keyShort, description, hasValue);
	}

	/**
	 * Queries the command line arguments map to determine if a given parameter
	 * was found on the command line.
	 * 
	 * @param param	the parameter name to query for.
	 * @return 		true if the command line parameter "param" was found on the
	 *         		command line, otherwise false.
	 */
	public boolean parameterExists(String param) {
		return itsCmdlineParameters.containsKey(param);
	}

	/**
	 * Queries the command line arguments output map, returning the value for a
	 * given key.
	 * 
	 * @param param		the parameter (or key) to look for.
	 * @return 			the value corresponding to the specified "param" parameter
	 *         			or null in the case of the parameter not existing.
	 */
	public String parameter(String param) {
		return itsCmdlineParameters.get(param);
	}

	/**
	 * @return the Ice Communicator.
	 */
	public Ice.Communicator communicator() {
		return itsCommunicator;
	}

	/**
	 * Parse the command line provided by the "args" parameter and populate
	 * itsCmdlineParameters with the found parameters.
	 * 
	 * @param args	the command line, tokenised and represented as space
	 *            	separated tokens.
	 */
	private void processCmdLineArgs(String args[]) {
		itsCmdlineParser.addOption("config", "c",
				"configuration parameter set file", true);
		itsCmdlineParser.addOption("log-config", "l",
				"logger configuration file", true);
		itsCmdlineParameters = itsCmdlineParser.parse(args);
	}

	/**
	 * Initialises the Log4J logging framework. This method takes a filename
	 * which can either be a properties file (i.e. key/value pairs) or an XML
	 * file. If the filename extension if ".xml" it will be parsed as an XML
	 * file, otherwise it is assumed to be a properties file.
	 * 
	 * If the parameter is null, it will look for a file called askap.log_cfg in
	 * the current working directory (and parse it as a properties file if it
	 * exists.
	 * 
	 * If neither a default file is found, or the file specified cannot be
	 * found/opened then a BasicConfigurator will provide a default logging
	 * configuration.
	 * 
	 * @param logcfg	log configuration filename.
	 */
	private void initLogging(String logcfg) {
		if (logcfg == null) {
			logcfg = "askap.log_cfg";
		}
		File f = new File(logcfg);
		if (f.exists()) {
			if (logcfg.endsWith(".xml")) {
				DOMConfigurator.configure(logcfg);
			} else {
				PropertyConfigurator.configure(logcfg);
			}
		} else {
			BasicConfigurator.configure();
		}

		String hostname = "<unknown>";
		try {
			hostname = InetAddress.getLocalHost().getHostName();
		} catch (UnknownHostException e) {
			// USe default hostname
		}

		MDC.put("hostname", hostname);
	}

	/**
	 * Reads the configuration file creating a ParameterSet (itsParset)
	 * 
	 * @param filename	the filename of the log configuration file to read.
	 */
	private void initConfig(String filename) {
		assert (filename != null);

		try {
			itsParset = new ParameterSet(filename);
		} catch (IOException e) {
			throw new RuntimeException("Failed to open configuration file: "
					+ filename);
		}
	}

	/**
	 * Initialise Ice communicator based on ice properties in the Parameter Set
	 * (itsParset)
	 */
	@SuppressWarnings("rawtypes")
	private void initIce() {
		// Create IceProperties
		Ice.Properties props = Ice.Util.createProperties();
		ParameterSet subset = config().subset("ice_properties.");
		for (Enumeration e = subset.keys(); e.hasMoreElements(); /**/) {
			String key = (String) e.nextElement();
			String value = subset.getProperty(key);
			props.setProperty(key, value);
		}

		// Create initialisation data
		Ice.InitializationData id = new Ice.InitializationData();
		id.properties = props;

		itsCommunicator = Ice.Util.initialize(id);
	}

	/**
	 * Shutdown Ice Communicator
	 */
	private void shutdownIce() {
		itsCommunicator.destroy();
	}
}
