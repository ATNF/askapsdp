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

import IceStorm.TopicManagerPrx;
import IceStorm.TopicPrx;
import askap.interfaces.logging.ILogEvent;
import askap.interfaces.logging.ILoggerPrx;
import askap.interfaces.logging.ILoggerPrxHelper;

import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.TimeUnit;

/**
 * The thread which manages the connection and sends messages to the
 * IceStorm topic.
 */
class IceLoggerThread extends Thread {
    /**
     * The maximum number of messages to enqueue if the connection is down.
     */
    private static final int MAX_BUFFER = 500;

    /**
     * Number of milliseconds to wait between IceStorm reconnect retries
     */
    private static final int RECONNECT_RETRY_INTERVAL = 1000;

    /**
     * The trigger to keep running or shut the thread down.
     */
    private boolean itsKeepRunning = true;

    /**
     * Name of the IceStorm topic.
     */
    private final String itsTopicName;

    /**
     * Identity of the TopicManager
     */
    private final String itsTopicManager;

    /**
     * The Ice Communicator.
     */
    private Ice.Communicator itsCommunicator;

    /**
     * The Ice object for sending log messages.
     */
    private ILoggerPrx itsLoggingService;

    /**
     * The queue of unsent messages.
     */
    private final LinkedBlockingDeque<ILogEvent> itsBuffer = new LinkedBlockingDeque<ILogEvent>(MAX_BUFFER);

    /**
     * Flag to indicate an error has been reported. This ensures an error is
     * only reported once, rather than every time a reconnect is retried
     */
    private boolean itsErrorReported = false;

    /**
     * Constructor
     *
     * @param host         hostname for the locator service
     * @param port         port for the locator service
     * @param topic        name of the topic to publsh log messages on
     * @param topicManager identity of the topic manager in the locator service
     */
    public IceLoggerThread(String host, String port, String topic, String topicManager) {
        itsTopicName = topic;
        itsTopicManager = topicManager;

        // Initialize a communicator with these properties.
        Ice.Properties props = Ice.Util.createProperties();
        props.setProperty("Ice.Default.Locator", "IceGrid/Locator:tcp -h "
                + host + " -p " + port);
        props.setProperty("Ice.IPv6", "0");

        Ice.InitializationData id = new Ice.InitializationData();
        id.properties = props;
        itsCommunicator = Ice.Util.initialize(id);
    }

    /**
     * Instructs the IceLoggerThread to exit.
     * This call is non-blocking.
     */
    public void shutdown() {
        itsKeepRunning = false;
        synchronized (itsBuffer) {
            itsBuffer.notify();
        }
    }

    /**
     * Main loop of checking the connection and sending messages.
     */
    public void run() {
        ILogEvent event = null;
        while (itsKeepRunning) {
            try {
                // Check if connection is up
                if (!isConnected()) {
                    // Try to connect
                    if (!connect()) {
                        // Wait a bit before attempting reconnection
                        Thread.sleep(RECONNECT_RETRY_INTERVAL);
                        continue;
                    }
                }

                // Get a log event
                event = itsBuffer.pollLast(500, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                // Faster polling or reconnecting is ok in the rare case
                // this happens
            }
            if (event == null) {
                continue;
            }

            // Try to send the next message
            try {
                itsLoggingService.send(event);
                event = null; // Has been sent
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
        if (itsCommunicator != null) {
            if (isConnected()) {
                itsCommunicator.shutdown();
                itsCommunicator.waitForShutdown();
            }
            itsCommunicator.destroy();
            itsCommunicator = null;
        }
    }

    /**
     * Submit a new message to be logged.
     *
     * This is non-blocking, since it is called from the application code we don't
     * want logging problems to hang the application.
     *
     * If the internal queue is full, the event will be discarded.
     */
    protected void submitLog(ILogEvent event) {
        // This is non-blocking. If the queue is full, it will simply return
        itsBuffer.offerFirst(event);
    }

    /**
     * Check if the connection to the service appears valid.
     *
     * @return True if connection is good, False if connection is down.
     */
    protected boolean isConnected() {
        return !(itsCommunicator == null || itsCommunicator.isShutdown()
                || itsLoggingService == null);
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
                    .stringToProxy(itsTopicManager);
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

        // Start reporting errors again
        itsErrorReported = false;

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
