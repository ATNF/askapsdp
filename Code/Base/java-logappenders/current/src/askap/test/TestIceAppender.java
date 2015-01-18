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
package askap.test;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * This test simply sends some messages via the Log4J IceAppender and ensures
 * they have been received. This test program acts as the Log Archiver, which
 * usually subscribes to he appropriate IceStorm topic and receives the
 * LogEvents.
 *
 * The test requires the presence of this file "TestIceAppender.log_cfg" which
 * would usually have the following contents:
 *
 * @code
 * log4j.rootLogger=DEBUG,REMOTE
 *
 * log4j.appender.REMOTE=IceAppender
 * log4j.appender.REMOTE.locator_host=localhost
 * log4j.appender.REMOTE.locator_port=4061
 * log4j.appender.REMOTE.topic=logger
 * @endcode
 */
public class TestIceAppender extends askap.interfaces.logging._ILoggerDisp {
    private static final long serialVersionUID = 1L;

    /**
     * Log events are raised on this object
     */
    private static final Logger log = Logger.getLogger(TestIceAppender.class);

    /**
     * Total number of log message to send
     */
    private static final int TOTAL = 100;

    /**
     * Count of received log messages
     */
    private static final AtomicInteger count = new AtomicInteger(0);

    /**
     * Callback method, called upon receipt of a new log message
     */
    public void send(askap.interfaces.logging.ILogEvent event, Ice.Current current) {
        System.out.println("Test: Got a message: " + event.message);
        count.incrementAndGet();
    }

    public static void main(String[] args) {
        // Setup ICE so this test program can subscribe to the logger topic
        Ice.Communicator ic = null;
        Ice.ObjectAdapter adapter = null;
        askap.interfaces.logging._ILoggerDisp logger = null;
        Ice.ObjectPrx proxy = null;
        IceStorm.TopicPrx topic = null;

        // Overall test status is -1 (fail) until the messages arrive
        int status = 1;

        try {
            System.out.println("Test: Initializing ICE");
            ic = Ice.Util.initialize(args);
            if (ic == null) {
                throw new RuntimeException("ICE Communicator initialisation failed");
            }

            // Obtain the topic or create
            IceStorm.TopicManagerPrx topicManager;

            System.out.println("Test: Obtaining TopicManager");
            Ice.ObjectPrx obj = ic.stringToProxy("IceStorm/TopicManager@IceStorm.TopicManager");
            topicManager = IceStorm.TopicManagerPrxHelper.checkedCast(obj);

            System.out.println("Test: Obtaining Logger Topic");
            String topicName = "logger";
            try {
                topic = topicManager.retrieve(topicName);
            } catch (IceStorm.NoSuchTopic e) {
                try {
                    topic = topicManager.create(topicName);
                } catch (IceStorm.TopicExists e1) {
                    // Do nothing
                }
            }

            // Create an adapter and subscribe
            adapter = ic.createObjectAdapter("TestIceAppenderAdapter");
            if (adapter == null) {
                throw new RuntimeException("ICE adapter initialisation failed");
            }

            logger = new TestIceAppender();
            proxy = adapter.addWithUUID(logger).ice_oneway();

            java.util.Map<String, String> qos = null;
            topic.subscribeAndGetPublisher(qos, proxy);
            adapter.activate();

            // Setup logging
            PropertyConfigurator.configure(args[0]);

            System.out.println("Test: Sending log messages");
            for (int i = 1; i <= TOTAL; i++) {
                log.debug("Debug   Message " + i);
                log.warn("Warning Message " + i);
                log.error("Error   Message " + i);
                Thread.sleep(20);
            }

            // Wait for the three messages, up to a maximum of 10 seconds
            final int WAIT_FOR = 10;
            for (int i = 0; i < WAIT_FOR; ++i) {
                if (count.get() == TOTAL * 3) {
                    // Success, got all messages expected
                    status = 0;
                    break;
                }
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }

        } catch (Exception e) {
            System.err.println(e.getMessage());
        } finally {
            // Cleanup ICE
            if (ic != null) {
                // Cleanup
                try {
                    adapter.deactivate();
                    topic.unsubscribe(proxy);
                    ic.shutdown();
                    ic.waitForShutdown();
                    ic.destroy();
                } catch (Exception e) {
                    System.err.println(e.getMessage());
                }
            }
        }

        System.out.println("Received " + count + " of expected " + TOTAL * 3 + " messages");
        System.exit(status);
    }
}
