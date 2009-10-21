package askap;

import org.apache.log4j.Logger;
import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;

public class TestIceAppender {
    private static final Logger log = Logger.getLogger(TestIceAppender.class);

    public static void main(String[] args) {
        PropertyConfigurator.configure(args[0]);

        log.debug("Debug   Message");
        log.warn ("Warning Message");
        log.error("Error   Message");
    }
}
