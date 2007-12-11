/// @file
/// @brief CONRAD logging 
/// 
/// This file provides logging for CONRAD. At this stage it just wraps around 
/// log4cxx. This is provided to make a move to a possible replacement logging
/// package easier. It can also be used to set-up loggers for  system where 
/// log4cxx can't be build.
///
/// @code
///  CONRADLOG_INIT("tLogging.log_cfg");
///  int i = 1;
///  CONRAD_LOGGER(locallog, ".test");
///
///  CONRADLOG_WARN(locallog,"Warning. This is a warning.");
///  CONRADLOG_INFO(locallog,"This is an automatic (subpackage) log");
///  CONRADLOG_INFO_STR(locallog,"This is " << i << " log stream test.");
/// 
/// @endcode
/// @author Malte Marquarding <Malte.Marquarding@csiro.au>

#ifndef CONRAD_LOGGING_H
#define CONRAD_LOGGING_H

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/mdc.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>

namespace conrad {


/// The package name defined by any CONRAD package
#ifndef CONRAD_PACKAGE_NAME
# define CONRAD_PACKAGE_NAME "unknown"
#endif

  /// Turn a logger name into a standardised version
  /// All loggers will be under the  base "conrad" logger package.
  /// @param inname the logger package name
  /// @li An empty inname will result in a logger conrad.CONRAD_PACKAGE_NAME
  /// @li An inname starting with "." will become 
  /// "conrad.CONRAD_PACKAGE_NAME.inname"
  /// @li If the inanme doesn't start with "." it will be under the conrad logger, 
  /// i.e. conrad.inname. This is used by the CONRAD_LOGGER macro.

  /// @return a standardised logger package name
  inline std::string generateLoggerName(const std::string& inname)
  {
    const std::string name("conrad.");
    if (inname.length() > 0) {
      if (inname[0] == '.') {
        std::cout << name << std::string(CONRAD_PACKAGE_NAME) << std::endl;
        return (name + std::string(CONRAD_PACKAGE_NAME) + inname);
      } else {
        return (name + inname);
      }
    }
    return (name + std::string(CONRAD_PACKAGE_NAME));
  }

/// Initialise a logger from a file. If none is specified or found, it uses the default settings
#define CONRADLOG_INIT(filename)                                        \
  do {                                                                  \
    if (std::string(filename).length() == 0) {                          \
      log4cxx::BasicConfigurator::configure();                          \
      break;                                                            \
    }                                                                   \
    if (!strstr(filename, ".log_cfg")) {                                \
      throw(conrad::ConradError("Logger configuration file needs suffix .log_cfg")); \
    } else {                                                            \
      std::fstream logis;                                               \
      logis.open(filename, std::ios::in);                               \
      if (logis.is_open()) {                                            \
        logis.close();                                                  \
        log4cxx::MDC::put("hostname", conrad::getHostName());           \
        log4cxx::PropertyConfigurator::configure(log4cxx::File(filename)); \
        break;                                                          \
      }                                                                 \
    }                                                                   \
    log4cxx::BasicConfigurator::configure();                            \
  } while(0)

 
/// Create a logger handle with a specific logger name. This needs to be called in ".cc" before any logging occurs. The logger name is genrated by generateLoggerName.
#define CONRAD_LOGGER(handle, name)                                     \
  static log4cxx::LoggerPtr handle = log4cxx::Logger::getLogger(conrad::generateLoggerName(std::string(name)));

  
/// Macros to generate a log message from a string
/// @name Logging macros 
// @{
/// debug
#define CONRADLOG_DEBUG(logger,message) ConradLog(logger,Debug,message)
/// info
#define CONRADLOG_INFO(logger,message) ConradLog(logger,Info,message)
/// warn
#define CONRADLOG_WARN(logger,message) ConradLog(logger,Warn,message)
/// error
#define CONRADLOG_ERROR(logger,message) ConradLog(logger,Error,message)
/// fatal
#define CONRADLOG_FATAL(logger,message) ConradLog(logger,Fatal,message)
// @}

/// Macros to generate a log message from a stream
/// @name Logging stream macros 
// @{
/// debug
#define CONRADLOG_DEBUG_STR(logger,stream) ConradLogStr(logger,Debug,stream)
/// info
#define CONRADLOG_INFO_STR(logger,stream) ConradLogStr(logger,Info,stream)
/// warn
#define CONRADLOG_WARN_STR(logger,stream) ConradLogStr(logger,Warn,stream)
/// error
#define CONRADLOG_ERROR_STR(logger,stream) ConradLogStr(logger,Error,stream)
/// fatal
#define CONRADLOG_FATAL_STR(logger,stream) ConradLogStr(logger,Fatal,stream)
// @}

/// Do the actual logging
#define ConradLog(logger,type,message)                                  \
  do {                                                                  \
    if (logger->is##type##Enabled()) {                                  \
      logger->forcedLog(log4cxx::Level::get##type(), message,           \
                        LOG4CXX_LOCATION);                              \
    }                                                                   \
  } while(0)

/// Do the actual logging of a stream
#define ConradLogStr(logger,type,stream)                                \
  do {                                                                  \
    if (logger->is##type##Enabled()) {                                  \
      std::ostringstream conrad_log_oss;                                \
      conrad_log_oss << stream;                                         \
      logger->forcedLog(log4cxx::Level::get##type(), conrad_log_oss.str(), \
                        LOG4CXX_LOCATION);                              \
    }                                                                   \
  } while(0)
  

} // end namespaces

#endif
