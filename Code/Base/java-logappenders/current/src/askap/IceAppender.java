/**
 * Copyright (c) 2009,2014 CSIRO
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
 */
package askap;

import java.net.InetAddress;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

import askap.interfaces.logging.LogLevel;
import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.Level;
import org.apache.log4j.spi.LoggingEvent;

/**
 * log4j appender for the ASKAP Ice logging system.
 * Logging is asynchronous, with the generation of the log event
 * and the act of publishing being decoupled by a buffer. The IceLoggerThread
 * consumes from this buffer and publishes to an IceStorm topic.
 */
public class IceAppender extends AppenderSkeleton {

    // Configuration options which should be set automatically by log4j
    // based on the contents of the log config file
    private volatile String itsLocatorPort;
    private volatile String itsLocatorHost;
    private volatile String itsTopic;
    private volatile String itsTopicManager = "IceStorm/TopicManager@IceStorm.TopicManager";
    private volatile String itsHostName;
    private volatile String itsTag;

    /**
     * Map all log4j log levels to ASKAP/ICE log levels so a log4j event can be
     * turned into an ASKAP LogEvent.
     */
    private final Map<Level, LogLevel> itsLevelMap = new HashMap<Level, LogLevel>();

    /**
     * The thread that does the backend sending over Ice.
     */
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

    /**
     * Called automatically by Log4j to set the "topic_manager" option.
     */
    public void settopic_manager(String topicmanager) {
        itsTopicManager = topicmanager;
    }

    public String gettopic_manager() {
        return itsTopicManager;
    }

    public boolean requiresLayout() {
        return false;
    }

    /**
     * Called once all the options have been set. This is where ICE can be
     * initialized and the topic created, since the configuration options have
     * now been set hence we know the locator host, locator port and logger
     * topic name.
     */
    public synchronized void activateOptions() {
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
        if (itsLevelMap.isEmpty()) {
            itsLevelMap.put(Level.TRACE, askap.interfaces.logging.LogLevel.TRACE);
            itsLevelMap.put(Level.DEBUG, askap.interfaces.logging.LogLevel.DEBUG);
            itsLevelMap.put(Level.INFO, askap.interfaces.logging.LogLevel.INFO);
            itsLevelMap.put(Level.WARN, askap.interfaces.logging.LogLevel.WARN);
            itsLevelMap.put(Level.ERROR, askap.interfaces.logging.LogLevel.ERROR);
            itsLevelMap.put(Level.FATAL, askap.interfaces.logging.LogLevel.FATAL);
        }

        itsIceLoggerThread = new IceLoggerThread(itsLocatorHost, itsLocatorPort,
                itsTopic, itsTopicManager);
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
            // milliseceonds
            iceevent.created = event.getTimeStamp() / 1000.0;
            iceevent.level = itsLevelMap.get(event.getLevel());
            iceevent.message = event.getRenderedMessage();
            iceevent.hostname = itsHostName;
            iceevent.tag = itsTag;

            // Submit for logging
            itsIceLoggerThread.submitLog(iceevent);
        }
    }

    public synchronized void close() {
        if (itsIceLoggerThread != null) {
            itsIceLoggerThread.shutdown();
        }
    }


    /**
     * Simple utility function to ensure the appropriate options have been set
     * in the configuration file.
     */
    private boolean verifyOptions() {
        final String error = "IceAppender: Cannot initialise - ";

        if (Objects.equals(itsLocatorHost, "")) {
            System.err.println(error + "locator host not specified");
            return false;
        } else if (Objects.equals(itsLocatorPort, "")) {
            System.err.println(error + "locator port not specified");
            return false;
        } else if (Objects.equals(itsTopic, "")) {
            System.err.println(error + "logging topic not specified");
            return false;
        } else {
            return true;
        }
    }
}