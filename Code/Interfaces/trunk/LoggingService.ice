module askap
{      
  module interfaces
  {
    module logging
    {
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
        string level;
        // the actual log message
        string message;
      };
      
      // a list of ILogEvents
      sequence<ILogEvent> eventlist;

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
        stringlist levels;
        int limit;
      };
      // The inteface for querying the logarchive
      interface ILogQuery
      {
        // query the logarchive database returning the list of matches
        idempotent eventlist query(IQueryObject q);
        // get the logger names (origin) with an optional name match
        idempotent stringlist getLoggers(string name);
        // get the availabel log levels
        idempotent stringlist getLevels();
      };
    };
  };
};
