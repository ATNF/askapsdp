package askap.logger;

import java.io.*;
import java.util.Properties;
import java.lang.reflect.*;

/**
 * Base class for ASKAP logger implementations.
 * 
 * <P>
 * This uses a configuration file which defines which concrete implementation to
 * instanciate and contains any settings required by that implementation. The
 * configuration file is read as a resource, the name of which can be specified by
 * defining the <tt>askap.logger.configuration</tt> property. By default the resource
 * <tt>askaplogger.properties</tt> will be used.
 * 
 * <P>
 * The fully qualified name of the concrete class to be used for logging must be specified
 * by a property called <tt>askap.logger.loggerclass</tt> within the configuration file.
 * 
 * <P>
 * Sub-classes must implement a constructor which takes a <tt>Properties</tt> as an
 * argument, which configures the logging service appropriately and creates the root
 * logger.
 * 
 * @author David Brodrick
 */
public abstract class Logger {
  /** The name of the property which specifies the configuration resource. */
  private static final String theirPropName = "askap.logger.configuration";

  /** The default name of the configuration resource. */
  private static final String theirConfigName = "askaplogger.properties";

  /** Reference to the root logger. */
  private static Logger theirLogger = null;

  // Static block which reads the configuration and configures logging
  static {
    String configname = System.getProperty(theirPropName, theirConfigName);
    InputStream configstream = ClassLoader.getSystemResourceAsStream(configname);
    if (configstream == null) {
      // Unable to find the configuration resource
      System.err.println("askap.logger.Logger: Error: Unable to locate configuration resource \"" + configname + "\"");
    } else {
      try {
        // Load the properties file and find name of concrete class to instanciate
        Properties config = new Properties();
        config.load(configstream);
        String loggerclass = config.getProperty("askap.logger.loggerclass");
        if (loggerclass == null) {
          System.err.println("askap.logger.Logger: Error: Configuration \"" + configname
                  + "\" doesn't specify a value for askap.logger.loggerclass");
        } else {
          // Try to instanciate the concrete implementation
          try {
            Constructor con = Class.forName(loggerclass).getConstructor(new Class[] { Properties.class });
            theirLogger = (Logger) (con.newInstance(new Object[] { config }));
          } catch (Exception f) {
            System.err.println("askap.logger.Logger: Error: Instanciating \"" + loggerclass + "\": " + f);
          }
        }
      } catch (IOException e) {
        System.err.println("askap.logger.Logger: Error: Reading configuration \"" + configname + "\": " + e);
      }
    }
  }

  /** Get the default/root Logger. */
  public static Logger getRootLogger() {
    return theirLogger;
  }

  /** Get a Logger with the specified name. */
  public static Logger getLogger(String name) {
    return theirLogger.getLogger_real(name);
  }

  /** Get a Logger with the specified name. */
  protected abstract Logger getLogger_real(String name);

  /** Log a trace level message. */
  public abstract void trace(String msg);

  /** Log a debug level message. */
  public abstract void debug(String msg);

  /** Log an info level message. */
  public abstract void info(String msg);

  /** Log a warn level message. */
  public abstract void warn(String msg);

  /** Log an error level message. */
  public abstract void error(String msg);

  /** Log a fatal level message. */
  public abstract void fatal(String msg);
  
  /** Simple test program. */
  public static void main(String[] args) {
    Logger l1 = getRootLogger();
    l1.trace("a trace message");
    l1.debug("a debug message");
    l1.info("an info message");
    l1.warn("a warn message");
    l1.error("an error message");
    l1.fatal("a fatal message");
    Logger l2 = getLogger("foobar");
    l2.trace("a trace message");
    l2.debug("a debug message");
    l2.info("an info message");
    l2.warn("a warn message");
    l2.error("an error message");
    l2.fatal("a fatal message");
  }
}
