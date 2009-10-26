package askap.logger;

import java.util.Properties;

import askap.interfaces.logging.*;
import java.util.Vector;

/**
 * Logger implementation which talks native Ice to the ASKAP Logging Service.
 * 
 * @author David Brodrick
 */
public class LoggerIce extends Logger {
  /** String for finding the locator service. */
  protected static String theirLocatorString;

  /** The Ice Communicator. */
  protected static Ice.Communicator theirCommunicator;

  /** The Ice object for sending log messages. */
  protected static ILoggerPrx theirLoggingService;

  /** The maximum number of messages to enqueue if the connection is down. */
  protected static int theirMaxBuffer = 100;

  /** The queue of unsent messages. */
  protected static Vector<ILogEvent> theirBuffer = new Vector<ILogEvent>();

  /** The name of this particular logger. */
  protected String itsName;

  /** Configure from properties and get the root Logger. */
  public LoggerIce(Properties props) {
    // Get the host and port for the locator service
    // String port = props.getProperty("askap.logger.ice.locator_port", theirDefaultPort);
    // String host = props.getProperty("askap.logger.ice.locator_host", theirDefaultHost);
    theirLocatorString = "IceGrid/Locator:tcp -h localhost -p 4061";
    itsName = "root";
  }

  /** Create a new Logger with the specified name. */
  protected LoggerIce(String name) {
    itsName = name;
  }

  /** Return a new Logger with the specified name. */
  protected Logger getLogger_real(String name) {
    return new LoggerIce(name);
  }

  /** Log a trace level message. */
  public void trace(String msg) {
    submitLog(itsName, msg, LogLevel.TRACE);
  }

  /** Log a debug level message. */
  public void debug(String msg) {
    submitLog(itsName, msg, LogLevel.DEBUG);
  }

  /** Log an info level message. */
  public void info(String msg) {
    submitLog(itsName, msg, LogLevel.INFO);
  }

  /** Log a warn level message. */
  public void warn(String msg) {
    submitLog(itsName, msg, LogLevel.WARN);
  }

  /** Log an error level message. */
  public void error(String msg) {
    submitLog(itsName, msg, LogLevel.ERROR);
  }

  /** Log a fatal level message. */
  public void fatal(String msg) {
    submitLog(itsName, msg, LogLevel.FATAL);
  }

  /** Submit a new message to be logged. */
  protected static void submitLog(String name, String msg, LogLevel level) {
    ILogEvent event = new ILogEvent();
    event.origin = name;
    event.message = msg;
    event.level = level;
    // event.created = ;
    if (theirBuffer.size() < theirMaxBuffer) {
      theirBuffer.add(event);
    }
  }

  /**
   * Check if the connection to the service appears valid.
   * 
   * @return True if connection is good, False if connection is down.
   */
  protected static boolean isConnected() {
    if (theirCommunicator == null || theirCommunicator.isShutdown()) {
      return false;
    } else {
      return true;
    }
  }

  /**
   * Try to connect to the logging service.
   * 
   * @return True if connection made, False if connection failed.
   */  
  protected static boolean connect() {
    theirCommunicator = Ice.Util.initialize();
    Ice.ObjectPrx base = theirCommunicator.stringToProxy(theirLocatorString);
    theirLoggingService = ILoggerPrxHelper.checkedCast(base);
    if (theirLoggingService == null) {
      return false;
    }
    return true;
  }

  /** The thread which manages the connection and send messages to the logging service. */
  protected class LoggerIceThread extends Thread {
    /** The trigger to keep running or shut the thread down. */
    private boolean itsKeepRunning = true;

    public void run() {
      while (itsKeepRunning) {
        try {
          // Wait for a new log message
          theirBuffer.wait();
        } catch (InterruptedException e) {
        }

        // Check if connection is up
        if (!isConnected()) {
          // Try to connect
          if (!connect()) {
            continue;
          }
        }

        // Send all messages in the buffer
        try {
          while (!theirBuffer.isEmpty()) {
            // Try to send the next message
            ILogEvent event = theirBuffer.get(0);
            theirLoggingService.send(event);
            theirBuffer.remove(0);
          }
        } catch (Exception e) {
          // An unexpected error, assume communications links is down
          try {
            theirCommunicator.shutdown();
          } catch (Exception f) {
          }
          theirCommunicator = null;
          theirLoggingService = null;
        }
      }
    }
  };
}
