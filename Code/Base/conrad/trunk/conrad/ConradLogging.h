/// @file
/// @brief CONRAD logging 
/// 
/// This file provides logging for CONRAD. At this stage it just wraps around 
/// log4cxx. This is provided to make a move to a possible replacement logging
/// package easier. It can also be used to set-up loggers for  system where 
/// log4cxx can't be build.
/// @author Malte Marquarding <Malte.Marquarding@csiro.au>

#ifndef CONRAD_LOGGING_H
#define CONRAD_LOGGING_H

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/ndc.h>

#include <sstream>
#include <fstream>
#include <iostream>

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>

namespace conrad {


#ifndef CONRAD_PACKAGE_NAME
# define CONRAD_PACKAGE_NAME "Unknown_package"
#endif

/// Initialise a logger from a file. If none is specified or found, it uses 
/// the default settings
#define CONRADLOG_INIT(filename)                                        \
  do {                                                                  \
    if (std::string(filename).length() == 0) {                          \
      log4cxx::BasicConfigurator::configure();                          \
      break;                                                            \
    }                                                                   \
    if (!strstr(filename, ".log_cfg")) {                                \
      throw(ConradError("Logger configuration file needs suffix .log_cfg")); \
    } else {                                                            \
      std::fstream logis;                                               \
      logis.open(filename, std::ios::in);                               \
      if (logis.is_open()) {                                            \
        logis.close();                                                  \
        log4cxx::NDC::push(CONRAD_PACKAGE_NAME);                        \
        log4cxx::NDC::push("["+getHostName() +  "]" );                  \
        log4cxx::PropertyConfigurator::configure(log4cxx::File(filename)); \
        break;                                                          \
      }                                                                 \
    }                                                                   \
    log4cxx::BasicConfigurator::configure();                            \
  } while(0)
  

/// Macro to create a log message
#define CONRADLOG_INFO(message) ConradLog(CONRAD_PACKAGE_NAME,Info,message)
#define CONRADLOG_INFO_STR(stream) ConradLogStr(CONRAD_PACKAGE_NAME,Info,stream)
#define CONRADLOG_WARN(message) ConradLog(CONRAD_PACKAGE_NAME,Warn,message)
#define CONRADLOG_WARN_STR(message) ConradLogStr(CONRAD_PACKAGE_NAME,Warn,message)
#define CONRADLOG_ERROR(message) ConradLog(CONRAD_PACKAGE_NAME,Error,message)
#define CONRADLOG_ERROR_STR(message) ConradLogStr(CONRAD_PACKAGE_NAME,Error,message)
#define CONRADLOG_FATAL(message) ConradLog(CONRAD_PACKAGE_NAME,Fatal,message)
#define CONRADLOG_FATAL_STR(message) ConradLogStr(CONRAD_PACKAGE_NAME,Fatal,message)

#define ConradLog(logpkg,type,message)                                  \
  do {                                                                  \
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(logpkg); \
    if (logger->is##type##Enabled()) {                                  \
      logger->forcedLog(log4cxx::Level::get##type(), message,           \
                        LOG4CXX_LOCATION);                              \
    }                                                                   \
  } while(0)

#define ConradLogStr(logpkg,type,stream)        \
  do {                                          \
    std::ostringstream conrad_log_oss;          \
    conrad_log_oss << stream;                   \
    ConradLog(logpkg,type, conrad_log_oss.str());       \
  } while(0)
  

} // end namespaces

#endif
