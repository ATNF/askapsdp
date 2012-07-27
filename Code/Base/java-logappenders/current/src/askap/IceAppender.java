/**
 * Copyright (c) 2009 CSIRO
 * Australia Telescope National Facility (ATNF)
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * PO Box 76, Epping NSW 1710, Australia
 * atnf-enquiries@csiro.au
 *
 * This file is part of the ASKAP software distribution.
 *
 * The ASKAP software distribution is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

package askap;

import java.util.*;
import java.net.InetAddress;
import org.apache.log4j.*;
import org.apache.log4j.spi.LoggingEvent;
import IceStorm.*;
import askap.interfaces.logging.*;

/**
 * log4j appender for the ASKAP Ice logging system.
 * Logging is asynchronous, with the generation of the log event
 * and the act of publishing being decoupled by a buffer. The IceLoggerThread
 * consumes from this buffer and publishes to an IceStorm topic.
 */
public class IceAppender extends AppenderSkeleton {
	/**
	 * The thread which manages the connection and sends messages to the
	 * IceStorm topic.
	 */
	protected static class IceLoggerThread extends Thread {
		/** The trigger to keep running or shut the thread down. */
		private boolean itsKeepRunning = true;

		/** Port for the locator service. */
		private String itsLocatorPort;

		/** Hostname for the locator service. */
		private String itsLocatorHost;

		/** Name of the IceStorm topic. */
		private String itsTopicName;

		/** The Ice Communicator. */
		protected Ice.Communicator itsCommunicator;

		/** The Ice object for sending log messages. */
		protected ILoggerPrx itsLoggingService;

		/** The maximum number of messages to enqueue if the connection is down. */
		protected int itsMaxBuffer = 500;

		/** The queue of unsent messages. */
		protected static LinkedList<ILogEvent> itsBuffer = new LinkedList<ILogEvent>();

		/**
		 * Flag to indicate an error has been reported. This ensures an error is
		 * only reported once, rather than every time a reconnect is retried
		 */
		private boolean itsErrorReported = false;

		public IceLoggerThread(String host, String port, String topic) {
			itsLocatorHost = host;
			itsLocatorPort = port;
			itsTopicName = topic;

			// Initialize a communicator with these properties.
			Ice.Properties props = Ice.Util.createProperties();
			props.setProperty("Ice.Default.Locator", "IceGrid/Locator:tcp -h "
					+ itsLocatorHost + " -p " + itsLocatorPort);

			Ice.InitializationData id = new Ice.InitializationData();
			id.properties = props;
			itsCommunicator = Ice.Util.initialize(id);
		}

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
							// Wait for a bit before attempting reconnection
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
						ILogEvent event = itsBuffer.pollFirst();
						if (event == null) {
							// pollFirst returns null in the case the list is
							// empty.
							// While this should not happen, protect against it.
							break;
						}
						itsLoggingService.send(event);
					}
				} catch (Exception e) {
					e.printStackTrace();
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
				itsCommunicator.waitForShutdown();
			}
			itsCommunicator.destroy();
			itsCommunicator = null;
		}

		/** Submit a new message to be logged. */
		protected void submitLog(ILogEvent event) {
			synchronized (itsBuffer) {
				if (itsBuffer.size() < itsMaxBuffer) {
					itsBuffer.addLast(event);
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
			if (itsCommunicator == null || itsCommunicator.isShutdown()
					|| itsLoggingService == null) {
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
			try {
				// Obtain the topic or create
				Ice.ObjectPrx obj = itsCommunicator
						.stringToProxy("IceStorm/TopicManager@IceStorm.TopicManager");
				TopicManagerPrx topicManager = IceStorm.TopicManagerPrxHelper
						.checkedCast(obj);
				
				TopicPrx topic = null;
				while (topic == null) {
					try {
						topic = topicManager.retrieve(itsTopicName);
					} catch (IceStorm.NoSuchTopic e1) {
						try {
							topic = topicManager.create(itsTopicName);
						} catch (IceStorm.TopicExists e2) {
							// Another client created the topic.
						}
					}
				}

				Ice.ObjectPrx pub = topic.getPublisher().ice_twoway();
				itsLoggingService = ILoggerPrxHelper.uncheckedCast(pub);
			} catch (Exception e) {
				reportErrorOnce("Error connecting to topic: " + e.getMessage());
				return false;
			}
			return true;
		}

		void reportErrorOnce(String msg) {
			if (!itsErrorReported) {
				System.err.println("IceLoggerThread: Failed to connect (" + msg + ")");
				System.err.println("IceLoggerThread: Will retry connect, however " +
						"no further connection errors will be reported");
				itsErrorReported = true;
			}
		}

	} // End class IceLoggerThread

	// Configuration options which should be set automatically by log4j
	// based on the contents of the log config file
	private String itsLocatorPort;
	private String itsLocatorHost;
	private String itsTopic;
	private String itsHostName;
	private String itsTag;

	/**
	 * Map all log4j log levels to ASKAP/ICE log levels so a log4j event can be
	 * turned into an ASKAP LogEvent.
	 */
	private HashMap<Level, LogLevel> itsLevelMap = new HashMap<Level, LogLevel>();

	/** The thread that does the backend sending over Ice. */
	private IceLoggerThread itsIceLoggerThread;

	/**
	 * Called automatically by Log4j to set the "Tag" option.
	 */
	public String getTag() {
		return itsTag;
	}

	public void setTag(String tag) {
		itsTag = tag;
	}

	/**
	 * Called automatically by Log4j to set the "locator_port" option.
	 */
	public void setlocator_port(String port) {
		itsLocatorPort = port;
	}

	public String getlocator_port() {
		return itsLocatorPort;
	}

	/**
	 * Called automatically by Log4j to set the "locator_host" option.
	 */
	public void setlocator_host(String host) {
		itsLocatorHost = host;
	}

	public String getlocator_host() {
		return itsLocatorHost;
	}

	/**
	 * Called automatically by Log4j to set the "topic" option.
	 */
	public void settopic(String topic) {
		itsTopic = topic;
	}

	public String gettopic() {
		return itsTopic;
	}

	public boolean requiresLayout() {
		return false;
	}

	/**
	 * Simple utility function to ensure the appropriate options have been set
	 * in the configuration file.
	 */
	private boolean verifyOptions() {
		final String error = "IceAppender: Cannot initialise - ";

		if (itsLocatorHost == "") {
			System.err.println(error + "locator host not specified");
			return false;
		} else if (itsLocatorPort == "") {
			System.err.println(error + "locator port not specified");
			return false;
		} else if (itsTopic == "") {
			System.err.println(error + "logging topic not specified");
			return false;
		} else {
			return true;
		}
	}

	/**
	 * Called once all the options have been set. This is where ICE can be
	 * initialized and the topic created, since the configuration options have
	 * now been set hence we know the locator host, locator port and logger
	 * topic name.
	 */
	public void activateOptions() {
		// First ensure host, port and topic are set
		if (!verifyOptions()) {
			return;
		}

		try {
			itsHostName = InetAddress.getLocalHost().getHostName();
		} catch (Exception e) {
			itsHostName = "unknown";
		}

		// Map all log4j log levels to ASKAP/ICE log levels so a log4j event can
		// be turned into an ASKAP LogEvent
		if (itsLevelMap.size() == 0) {
			itsLevelMap.put(Level.TRACE,
					askap.interfaces.logging.LogLevel.TRACE);
			itsLevelMap.put(Level.DEBUG,
					askap.interfaces.logging.LogLevel.DEBUG);
			itsLevelMap.put(Level.INFO, askap.interfaces.logging.LogLevel.INFO);
			itsLevelMap.put(Level.WARN, askap.interfaces.logging.LogLevel.WARN);
			itsLevelMap.put(Level.ERROR,
					askap.interfaces.logging.LogLevel.ERROR);
			itsLevelMap.put(Level.FATAL,
					askap.interfaces.logging.LogLevel.FATAL);
		}

		itsIceLoggerThread = new IceLoggerThread(itsLocatorHost,
				itsLocatorPort, itsTopic);
		itsIceLoggerThread.start();
	}

	/**
	 * Submit the log message to be sent to the IceStorm topic.
	 */
	public synchronized void append(LoggingEvent event) {
		if (itsIceLoggerThread != null) {
			// Create the payload
			askap.interfaces.logging.ILogEvent iceevent = new askap.interfaces.logging.ILogEvent();
			iceevent.origin = event.getLoggerName();

			// The ASKAPsoft log archiver interface expects Unix time in seconds
			// (the parameter is a double precision float) where log4j returns
			// millisec
			iceevent.created = event.getTimeStamp() / 1000.0;
			iceevent.level = itsLevelMap.get(event.getLevel());
			iceevent.message = event.getRenderedMessage();
			iceevent.hostname = itsHostName;
			iceevent.tag = this.itsTag;

			// Submit for logging
			itsIceLoggerThread.submitLog(iceevent);
		}
	}

	public synchronized void close() {
		if (itsIceLoggerThread != null) {
			itsIceLoggerThread.shutdown();
		}
	}
}
