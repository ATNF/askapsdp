package askap;

import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.spi.LoggingEvent;
import org.apache.log4j.spi.ErrorCode;
import org.apache.log4j.Layout;
import org.apache.log4j.Level;
import org.apache.log4j.helpers.LogLog;

import java.util.HashMap;


public class IceAppender extends AppenderSkeleton
{
    private String itsLocatorPort;
    private String itsLocatorHost;
    private String itsTopic;
    private Ice.Communicator itsIceComm = null;
    private askap.interfaces.logging.ILoggerPrx itsLogService;
    private HashMap<org.apache.log4j.Level,askap.interfaces.logging.LogLevel> itsLevelMap =
        new HashMap<org.apache.log4j.Level,askap.interfaces.logging.LogLevel>();

    public void setlocator_port(String port) {
        this.itsLocatorPort = port;
    }

    public String getlocator_port() {
        return this.itsLocatorPort;
    }

    public void setlocator_host(String host) {
        this.itsLocatorHost = host;
    }

    public String getlocator_host() {
        return this.itsLocatorHost;
    }

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
     * Called once all the options have been set. Starts
     *  listening for clients on the specified socket.
     */
    public void activateOptions()
    {
        // First ensure host, port and topic are set
        if (!verifyOptions()) {
            return;
        }

        if (itsLevelMap.size() == 0) {
            itsLevelMap.put(Level.TRACE, askap.interfaces.logging.LogLevel.TRACE);
            itsLevelMap.put(Level.DEBUG, askap.interfaces.logging.LogLevel.DEBUG);
            itsLevelMap.put(Level.INFO, askap.interfaces.logging.LogLevel.INFO);
            itsLevelMap.put(Level.WARN, askap.interfaces.logging.LogLevel.WARN);
            itsLevelMap.put(Level.ERROR, askap.interfaces.logging.LogLevel.ERROR);
            itsLevelMap.put(Level.FATAL, askap.interfaces.logging.LogLevel.FATAL);
        }


        // Initialize Ice
        // Get the initialized property set.
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
        } catch (Ice.ConnectionRefusedException e) {
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
            // (the parameter is a double precision float) where log4cxx returns
            // microseconds.
            iceevent.created = event.getTimeStamp() / 1000.0 / 1000.0;
            iceevent.level = itsLevelMap.get(event.getLevel());
            iceevent.message =  this.layout.format(event);

            // Send
            itsLogService.send(iceevent);
        }

    }

    public synchronized void close()
    {
    }
}
