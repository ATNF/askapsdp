/**
 * @file tIceAppender.cc
 *
 * @detail
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
 *
 * @copyright (c) 2009 CSIRO
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
package askap.test;

import org.apache.log4j.Logger;
import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;

public class TestIceAppender extends askap.interfaces.logging._ILoggerDisp {
    private static final Logger log = Logger.getLogger(TestIceAppender.class);
    static public int count = 0;

    public void send(askap.interfaces.logging.ILogEvent event, Ice.Current current) {
        System.out.println("Test: Got a message: " + event.message);
        count++;
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
            Ice.ObjectPrx obj = ic.stringToProxy("IceStorm/TopicManager");
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

            java.util.Map qos = null;
            topic.subscribeAndGetPublisher(qos, proxy);
            adapter.activate();

            // Setup logging
            PropertyConfigurator.configure(args[0]);

            System.err.println("Test: Sending log messages");
            for (int i = 0; i < 100; i++) {
                log.debug("Debug   Message");
                log.warn ("Warning Message");
                log.error("Error   Message");
            }

            // Wait for the three messages, up to a maximum of 5 seconds
            for (int i = 0; i < 5; ++i) {
                if (count == 300) {
                    // Success, got all messages expected
                    status = 0;
                    break;
                }
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {}
            }

        } catch (Ice.LocalException e) {
            e.printStackTrace();
            System.exit(1);
        } catch (Exception e) {
            System.err.println(e.getMessage());
            System.exit(1);
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
                    System.exit(1);
                }
            }
        }
        System.exit(status);
    }
}
