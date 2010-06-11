package askap.logger;

import java.util.Properties;

/**
 * ASKAP Logger implementation which uses log4j.
 * 
 * <P>
 * log4j will be configured using the properties found in the resources file referred to
 * by the <tt>askap.logging.configuration</tt> property.
 * 
 * @author David Brodrick
 */
public class LoggerLog4J extends Logger {
  /** The log4j Logger used by this instance. */
  protected org.apache.log4j.Logger itsLogger;

  /**
   * Perform static initialisation, using the given properties for configuration and set
   * the root logger.
   * 
   * @param props The properties to use for configuration.
   */
  public static void configure(Properties props) {
    org.apache.log4j.PropertyConfigurator.configure(props);
    theirRootLogger = new LoggerLog4J(org.apache.log4j.Logger.getRootLogger());
  }

  /** Shutdown logging. */
  public static void shutdown() {
    org.apache.log4j.LogManager.shutdown();
  }  
  
  /** Create a new logger using the specified name. */
  private LoggerLog4J(org.apache.log4j.Logger logger) {
    itsLogger = logger;
  }

  /** Create a new logger using the specified name. */
  public LoggerLog4J(String name) {
    itsLogger = org.apache.log4j.Logger.getLogger(name);
  }

  /** Log a trace level message. */
  public void trace(String msg) {
    itsLogger.trace(msg);
  }

  /** Log a debug level message. */
  public void debug(String msg) {
    itsLogger.debug(msg);
  }

  /** Log an info level message. */
  public void info(String msg) {
    itsLogger.info(msg);
  }

  /** Log a warn level message. */
  public void warn(String msg) {
    itsLogger.warn(msg);
  }

  /** Log an error level message. */
  public void error(String msg) {
    itsLogger.error(msg);
  }

  /** Log a fatal level message. */
  public void fatal(String msg) {
    itsLogger.fatal(msg);
  }
}
