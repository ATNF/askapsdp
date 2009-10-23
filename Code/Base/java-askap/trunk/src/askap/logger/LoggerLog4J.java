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
  private org.apache.log4j.Logger itsLogger;

  /** Configure log4j and get the root Logger. */
  public LoggerLog4J(Properties props) {
    org.apache.log4j.PropertyConfigurator.configure(props);
    itsLogger = org.apache.log4j.Logger.getRootLogger();
  }

  /** Create a new logger using the specified log4j Logger. */
  public LoggerLog4J(org.apache.log4j.Logger logger) {
    itsLogger = logger;
  }

  /** Get a Logger with the specified name. */
  protected Logger getLogger_real(String name) {
    return new LoggerLog4J(org.apache.log4j.Logger.getLogger(name));
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
