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

// Log4J imports
import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.spi.LoggingEvent;
import org.apache.log4j.spi.ErrorCode;
import org.apache.log4j.Layout;
import org.apache.log4j.Level;
import org.apache.log4j.helpers.LogLog;

// Standard Java imports
import java.util.HashMap;

public class IceAppender extends AppenderSkeleton
{
    // Configuration options which should be set automatically by log4j
    // based on the contents of the log config file
    private String itsLocatorPort;
    private String itsLocatorHost;
    private String itsTopic;

    // ICE communicator
    private Ice.Communicator itsIceComm = null;

    // Proxy to the logger
    private askap.interfaces.logging.ILoggerPrx itsLogService;

    // Map all log4j log levels to ASKAP/ICE log levels so a log4j event can be
    // turned into an ASKAP LogEvent
    private HashMap<org.apache.log4j.Level,askap.interfaces.logging.LogLevel> itsLevelMap =
        new HashMap<org.apache.log4j.Level,askap.interfaces.logging.LogLevel>();

    /**
     * Called automatically by Log4j to set the "locator_port"
     * option.
     */
    public void setlocator_port(String port) {
        this.itsLocatorPort = port;
    }

    public String getlocator_port() {
        return this.itsLocatorPort;
    }

    /**
     * Called automatically by Log4j to set the "locator_host"
     * option.
     */
    public void setlocator_host(String host) {
        this.itsLocatorHost = host;
    }

    public String getlocator_host() {
        return this.itsLocatorHost;
    }

    /**
     * Called automatically by Log4j to set the "topic"
     * option.
     */
    public void settopic(String topic) {
        this.itsTopic = topic;
    }

    public String gettopic() {
        return this.itsTopic;
    }

    public boolean requiresLayout()
    {
        return false;
    }

    /**
     * Simple utility function to ensure the appropriate options
     * have been set in the configuration file.
     */
    private boolean verifyOptions()
    {
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
     * Called once all the options have been set.
     * This is where ICE can be initialized and the topic created, since
     * the configuration options have now been set hence we know the 
     * locator host, locator port and logger topic name.
     */
    public void activateOptions()
    {
        // First ensure host, port and topic are set
        if (!verifyOptions()) {
            return;
        }

        // Map all log4j log levels to ASKAP/ICE log levels so a log4j event can be
        // turned into an ASKAP LogEvent
        if (itsLevelMap.size() == 0) {
            itsLevelMap.put(Level.TRACE, askap.interfaces.logging.LogLevel.TRACE);
            itsLevelMap.put(Level.DEBUG, askap.interfaces.logging.LogLevel.DEBUG);
            itsLevelMap.put(Level.INFO, askap.interfaces.logging.LogLevel.INFO);
            itsLevelMap.put(Level.WARN, askap.interfaces.logging.LogLevel.WARN);
            itsLevelMap.put(Level.ERROR, askap.interfaces.logging.LogLevel.ERROR);
            itsLevelMap.put(Level.FATAL, askap.interfaces.logging.LogLevel.FATAL);
        }


        // Initialize Ice
        Ice.Properties props = Ice.Util.createProperties();

        // Syntax example for the Ice.Default.Locator property:
        // "IceGrid/Locator:tcp -h localhost -p 4061"
        String locator =  "IceGrid/Locator:tcp -h " + itsLocatorHost + " -p " + itsLocatorPort;
        props.setProperty("Ice.Default.Locator", locator);

        // Initialize a communicator with these properties.
        Ice.InitializationData id = new Ice.InitializationData();
        id.properties = props;
        itsIceComm = Ice.Util.initialize(id);

        if (itsIceComm == null) {
            throw new RuntimeException("Ice.Communicatornot initialised. Terminating.");
        }

        // Obtain the topic or create
        IceStorm.TopicManagerPrx topicManager;

        try {
            Ice.ObjectPrx obj = itsIceComm.stringToProxy("IceStorm/TopicManager");
            topicManager = IceStorm.TopicManagerPrxHelper.checkedCast(obj);
        } catch (Ice.LocalException e) {
            System.err.println("Ice connection refused, messages will not be send to the log server");

            // Just return, treat this as non-fatal for the app, even though it
            // is fatal for logging via Ice.
            return;
        }

        IceStorm.TopicPrx topic = null;
        try {
            topic = topicManager.retrieve(itsTopic);
        } catch (IceStorm.NoSuchTopic e) {
            try {
                topic = topicManager.create(itsTopic);
            } catch (IceStorm.TopicExists e1) {
                // Do nothing
            }
        }

        // Get a handle to the logger proxy for this topic, this handle is then
        // used to publish log events
        Ice.ObjectPrx pub = topic.getPublisher().ice_oneway();
        itsLogService = askap.interfaces.logging.ILoggerPrxHelper.uncheckedCast(pub);
    }

    /**
     * Actually do the logging. The AppenderSkeleton's 
     *  doAppend() method calls append() to do the
     *  actual logging after it takes care of required
     *  housekeeping operations.
     */
    public synchronized void append(LoggingEvent event)
    {
        if (itsIceComm != null && itsIceComm.isShutdown()) {
            System.err.println("Ice is shutdown, cannot send log message");
            return;
        }

        if (itsLogService != null) {
            // Create the payload
            askap.interfaces.logging.ILogEvent iceevent = new askap.interfaces.logging.ILogEvent();
            iceevent.origin = event.getLoggerName();

            // The ASKAPsoft log archiver interface expects Unix time in seconds
            // (the parameter is a double precision float) where log4j returns
            // microseconds.
            iceevent.created = event.getTimeStamp() / 1000.0 / 1000.0;
            iceevent.level = itsLevelMap.get(event.getLevel());
            iceevent.message = event.getRenderedMessage();

            // Send
            itsLogService.send(iceevent);
        }
    }

    public synchronized void close()
    {
    }
}
