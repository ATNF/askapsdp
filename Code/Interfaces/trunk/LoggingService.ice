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

module askap
{
  module interfaces
  {
    module logging
    {

      enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL};
      // Standard string list
      sequence<string> stringlist;

      // The LogEvent representation
      struct ILogEvent
      {
        // the  logger name e.g. askap.scimath
        string origin;
        // the creation time of the log event (POSIX timestamp)
        double created;
        // The severity of the log event
        LogLevel level;
        // the actual log message
        string message;
      };

      // a list of ILogEvents
      sequence<ILogEvent> eventlist;
      // a list of LogLevels
      sequence<LogLevel> levellist;

      // The interface to implement when handling LogEvents
      interface ILogger
      {
        void send(ILogEvent event);
      };

      // The query argument object
      struct IQueryObject
      {
        string origin;
        string datemin;
        string datemax;
        levellist levels;
        int limit;
      };
      // The inteface for querying the logarchive
      interface ILogQuery
      {
        // query the logarchive database returning the list of matches
        idempotent eventlist query(IQueryObject q);
        // get the logger names (origin) with an optional name match
        idempotent stringlist getLoggers(string name);
        // get the available log levels
        idempotent stringlist getLevels();
      };
    };
  };
};

#endif
