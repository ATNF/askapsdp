Logging Documentation
=====================

The ASKAPsoft HPC applications are written in C/C++ and use the `Apache log4cxx`_ logging
framework. This allows flexible logging to stdout, file, or other sinks.

 .. _Apache log4cxx: https://logging.apache.org/log4cxx/

Here is an example of a typical log output::

    INFO  synthesis.parallel (0, nid6) [2013-12-12 17:32:41] - ASKAP cimager (parallel) running on 913 nodes
    INFO  synthesis.parallel (0, nid6) [2013-12-12 17:32:41] - askapparallel; ASKAPsoft==trunk; r20475; 2013-11-25
    INFO  synthesis.parallel (0, nid6) [2013-12-12 17:32:41] - Compiled without OpenMP support

The information, from left to right includes:

* Log level
* Origin of the log message
* MPI rank
* Hostname of the node the process is running on
* Date and time log message was created
* Log message

The log levels are enumerated and described below:

+---------+----------------------------------------------------------------------------+
|*Level*  |*Description*                                                               |
+=========+============================================================================+
| FATAL   | Indicates a problem which will result in an error condition under which    |
|         | process cannot continue                                                    |
+---------+----------------------------------------------------------------------------+
| ERROR   | Indicates a problem which results in seriously degraded functionality      |
+---------+----------------------------------------------------------------------------+
| WARN    | Indicates a problem which results in only slightly degraded functionality  |
+---------+----------------------------------------------------------------------------+
| INFO    | General information useful for a user monitoring the process               |
+---------+----------------------------------------------------------------------------+
| DEBUG   | Log messages useful to hardware or software engineers for debugging        |
|         | purposes                                                                   |
+---------+----------------------------------------------------------------------------+
| TRACE   | Very high level of debugging needed for debugging. E.g. Entry/Exit of a    |
|         | function call                                                              |
+---------+----------------------------------------------------------------------------+

All programs have a default logging configuration, essentially writing to STDOUT. You can customise this
using a custom log configuration file. For example create the following file, call it *askap.log_cfg*::

    # Configure the rootLogger
    log4j.rootLogger=INFO,STDOUT

    log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender
    log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout
    log4j.appender.STDOUT.layout.ConversionPattern=%-5p %c{2} (%X{mpirank}, %X{hostname}) [%d] - %m%n

Then use the "-l" parameter of the program to tell the program to use your log configuration file
instead of the default. E.g::

    cimager -c config.in -l askap.log_cfg

The log configuration file above says to log to the console (i.e. STDOUT) however only log
messages with level INFO or above (i.e. between INFO and FATAL).
