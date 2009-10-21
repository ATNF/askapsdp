package askap;

import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.spi.LoggingEvent;
import org.apache.log4j.spi.ErrorCode;
import org.apache.log4j.Layout;
import org.apache.log4j.helpers.LogLog;

public class IceAppender extends AppenderSkeleton
{
    private int locator_port = -1;
    private String locator_host;
    private String topic;

    public void setlocator_port(int port) {
        this.locator_port = port;
    }

    public int  getlocator_port() {
        return this.locator_port;
    }

    public void setlocator_host(String host) {
        this.locator_host = host;
    }

    public String getlocator_host() {
        return this.locator_host;
    }

    public void settopic(String topic) {
        this.topic = topic;
    }

    public String gettopic() {
        return this.topic;
    }


    public boolean requiresLayout()
    {
        return false;
    }

    /**
     * Called once all the options have been set. Starts
     *  listening for clients on the specified socket.
     */
    public void activateOptions()
    { 
    }

    /**
     * Actually do the logging. The AppenderSkeleton's 
     *  doAppend() method calls append() to do the
     *  actual logging after it takes care of required
     *  housekeeping operations.
     */
    public synchronized void append( LoggingEvent event )
    {   
    }

    public synchronized void close()
    {
    }
}

