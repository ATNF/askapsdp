module askap
{      
  module logging
  {
    module interfaces
    {
      struct LogEvent
      {
        string origin;
        double created;
        string level;
        string message;
      };
      interface ILogger
      {
        void send(LogEvent event);
      };
    };
  };
};
