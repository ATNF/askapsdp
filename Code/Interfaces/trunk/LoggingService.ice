module askap
{      
  module logging
  {
    module interfaces
    {
      struct ILogEvent
      {
        string origin;
        double created;
        string level;
        string message;
      };
      interface ILogger
      {
        void send(ILogEvent event);
      };
      sequence<ILogEvent> eventlist;
      sequence<string> strlist;
      struct IQueryObject
      {
        string origin;
        string datemin;
        string datemax;
        strlist levels;
        int limit;
      };
      interface ILogQuery
      {
        idempotent eventlist query(IQueryObject q);
        idempotent strlist getloggers(string name);
        idempotent strlist getlevels();
      };
    };
  };
};
