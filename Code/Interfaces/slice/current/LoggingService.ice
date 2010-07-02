// @file LoggingService.ice
//
// @copyright (c) 2009 CSIRO
// Australia Telescope National Facility (ATNF)
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// PO Box 76, Epping NSW 1710, Australia
// atnf-enquiries@csiro.au
//
// This file is part of the ASKAP software distribution.
//
// The ASKAP software distribution is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

#ifndef ASKAP_LOGGINGSERVICE_ICE
#define ASKAP_LOGGINGSERVICE_ICE

#include <CommonTypes.ice>

module askap
{

module interfaces
{

module logging
{
    /**
     * Exception to be thrown when a query is invalid.
     **/
    exception LogQueryException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Log level representation in order ov severity
     * This should be translated to the equivalent levels
     * in the logging system specific implementations.
     * These are the only valid levels.
     **/
    enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL};


    /**
     * The LogEvent representation/container. This is the message which gets
     * sent and returned from the archive.
     **/
    struct ILogEvent
    {
        // the logger name reflecting the software package e.g. askap.scimath
        string origin;
        // the creation time of the log event (POSIX timestamp)
        double created;
        // The severity of the log event
        LogLevel level;
        // the actual log message
        string message;
        // an optional tag to group log messages across log origins (different
        // packages). This can be a simple lower case string e.g. "antenna1" or
        // a comma-separated string "antenna1,subarray1"
        string tag;
        // the (optional) host name where the log message originated from
        string hostname;
    };

    /** a list of ILogEvents **/
    sequence<ILogEvent> EventSeq;

    /** a list of LogLevel **/
    sequence<LogLevel> LogLevelSeq;

    /** The interface to implement when handling LogEvents **/
    interface ILogger
    {
	    // Can't throw here if we wan't to use iceOneWay
	    // as Exceptions need two-way communication
        void send(ILogEvent event);
    };

    /**
     * The container for query parameters
     **/
    struct IQueryObject
    {
        // The log origin/logger name , e.g. 'askap.test'
	// SQL like match
        string origin;

        // The minimum date/time  as POSIX time double
        double datemin;

        // The minimum date/time  as POSIX time double
        double datemax;

        // The log levels to query on
        LogLevelSeq levels;

        // query by tag
	// SQL like match
        string tag;

        // query by hostname
	// SQL like match
        string hostname;

        // The maximum number of ILogEvents (rows) to return
        int limit;

	// The row number offset at
	int offset;
    };

    /**
     * The interface which defines a query of to the log archive.
     **/
    interface ILogQuery
    {
        // Query the logarchive database returning the list of matches
	// ordered by ascending date
        idempotent EventSeq query(IQueryObject q) throws LogQueryException;

        // Get the logger names (origin) with an optional name to match
        idempotent askap::interfaces::StringSeq getLoggers(string name);

        // Get the log message tags with an optional pattern to match
        idempotent askap::interfaces::StringSeq getTags(string pattern);

        // Get the log message hostnames with an optional pattern to match
        idempotent askap::interfaces::StringSeq getHostnames(string pattern);

        // Get the available log levels from the LogLevel enum as strings
        idempotent askap::interfaces::StringSeq getLevels();
    };
};

};

};

#endif
