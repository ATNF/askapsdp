package askap.logger;

import java.util.Properties;
import java.util.Vector;
import askap.interfaces.logging.*;

/**
 * Logger implementation which talks native Ice to the ASKAP Logging Service.
 * 
 * <P>
 * This expects the configuration to define a property named <i>askap.logger.ice.service</i>
 * which specifies the string used to find the Logging Service, eg,
 * <tt>IceGrid/Locator:tcp -h localhost -p 4061</tt>.
 * 
 * @author David Brodrick
 */
public class LoggerIce extends Logger {
  /** The property used to get the service location string. */
  protected static final String theirServiceProp = "askap.logger.ice.service"; 
  
  /** Default location for the service. */
  protected static final String theirDefaultService = "IceGrid/Locator:tcp -h localhost -p 4061";

  /** String for finding the locator service. */
  protected static String theirServiceString;

  /** The thread that does the actual logging in the background. */
  protected static LoggerIceThread theirLoggerThread;

  /** The name of this particular logger. */
  protected String itsName;

  /**
   * Perform static initialisation, using the given properties for configuration and set
   * the root logger.
   * 
   * @param props The properties to use for configuration.
   */
  public static void configure(Properties props) {
    theirServiceString = props.getProperty(theirServiceProp, theirDefaultService);
    theirLoggerThread = new LoggerIceThread();
    theirLoggerThread.start();
    theirRootLogger = new LoggerIce("root");
  }
  
  /** Shutdown logging. */
  public static void shutdown() {
    theirLoggerThread.shutdown();
  }

  /** Create a new Logger with the specified name. */
  public LoggerIce(String name) {
    itsName = name;
  }

  /** Log a trace level message. */
  public void trace(String msg) {
    theirLoggerThread.submitLog(itsName, msg, LogLevel.TRACE);
  }

  /** Log a debug level message. */
  public void debug(String msg) {
    theirLoggerThread.submitLog(itsName, msg, LogLevel.DEBUG);
  }

  /** Log an info level message. */
  public void info(String msg) {
    theirLoggerThread.submitLog(itsName, msg, LogLevel.INFO);
  }

  /** Log a warn level message. */
  public void warn(String msg) {
    theirLoggerThread.submitLog(itsName, msg, LogLevel.WARN);
  }

  /** Log an error level message. */
  public void error(String msg) {
    theirLoggerThread.submitLog(itsName, msg, LogLevel.ERROR);
  }

  /** Log a fatal level message. */
  public void fatal(String msg) {
    theirLoggerThread.submitLog(itsName, msg, LogLevel.FATAL);
  }

  /** The thread which manages the connection and send messages to the logging service. */
  protected static class LoggerIceThread extends Thread {
    /** The trigger to keep running or shut the thread down. */
    private boolean itsKeepRunning = true;

    /** The Ice Communicator. */
    protected Ice.Communicator itsCommunicator;

    /** The Ice object for sending log messages. */
    protected ILoggerPrx itsLoggingService;

    /** The maximum number of messages to enqueue if the connection is down. */
    protected int itsMaxBuffer = 500;

    /** The queue of unsent messages. */
    protected static Vector<ILogEvent> itsBuffer = new Vector<ILogEvent>();

    /** Make the main thread exit. */
    public void shutdown() {
      itsKeepRunning = false;
      synchronized (itsBuffer) {
        itsBuffer.notify();
      }
    }
    
    /** Main loop of checking the connection and sending messages. */
    public void run() {
      while (itsKeepRunning) {
        try {
          synchronized (itsBuffer) {
            if (itsBuffer.isEmpty()) {
              // Wait for a new log message
              itsBuffer.wait();
            } else {
              itsBuffer.wait(1000);
            }
          }
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
          while (!itsBuffer.isEmpty()) {
            // Try to send the next message
            ILogEvent event = itsBuffer.get(0);
            itsLoggingService.send(event);
            itsBuffer.remove(0);
          }
        } catch (Exception e) {
          // An unexpected error, assume communications links is down
          try {
            itsCommunicator.shutdown();
          } catch (Exception f) {
          }
          itsCommunicator = null;
          itsLoggingService = null;
        }
      }
      
      // Final cleanup
      if (isConnected()) {
        itsCommunicator.shutdown();
      }
    }

    /** Submit a new message to be logged. */
    protected void submitLog(String name, String msg, LogLevel level) {
      synchronized (itsBuffer) {
        if (itsBuffer.size() < itsMaxBuffer) {
          ILogEvent event = new ILogEvent();
          event.origin = name;
          event.message = msg;
          event.level = level;
          event.created = System.currentTimeMillis() / 1000.0d;
          itsBuffer.add(event);
          itsBuffer.notify();
        }
      }
    }

    /**
     * Check if the connection to the service appears valid.
     * 
     * @return True if connection is good, False if connection is down.
     */
    protected boolean isConnected() {
      if (itsCommunicator == null || itsCommunicator.isShutdown()) {
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
    protected boolean connect() {
      boolean res = false;
      try {
        itsCommunicator = Ice.Util.initialize();
        Ice.ObjectPrx base = itsCommunicator.stringToProxy(theirServiceString);
        itsLoggingService = ILoggerPrxHelper.checkedCast(base);
        if (itsLoggingService == null) {
          itsCommunicator.shutdown();
          itsCommunicator = null;
        } else {
          res = true;
        }
      } catch (Exception e) {
        itsCommunicator = null;
        itsLoggingService = null;
        res = false;
      }
      return res;
    }
  };
}
