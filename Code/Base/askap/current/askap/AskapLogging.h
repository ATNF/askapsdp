/// @file
/// @brief ASKAP logging
///
/// This file provides logging for ASKAP. At this stage it just wraps around
/// log4cxx. This is provided to make a move to a possible replacement logging
/// package easier. It can also be used to set-up loggers for  system where
/// log4cxx can't be build.
///
/// @code
///  ASKAPLOG_INIT("tLogging.log_cfg");
///  int i = 1;
///  ASKAP_LOGGER(locallog, ".test");
///
///  ASKAPLOG_WARN(locallog,"Warning. This is a warning.");
///  ASKAPLOG_INFO(locallog,"This is an automatic (subpackage) log");
///  ASKAPLOG_INFO_STR(locallog,"This is " << i << " log stream test.");
///
/// @endcode
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Malte Marquarding <Malte.Marquarding@csiro.au>

#ifndef ASKAP_LOGGING_H
#define ASKAP_LOGGING_H

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/mdc.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

namespace askap {


/// The package name defined by any ASKAP package
#ifndef ASKAP_PACKAGE_NAME
# define ASKAP_PACKAGE_NAME "unknown"
#endif

  /// Turn a logger name into a standardised version
  /// All loggers will be under the  base "askap" logger package.
  /// @param inname the logger package name
  /// @li An empty inname will result in a logger askap.ASKAP_PACKAGE_NAME
  /// @li An inname starting with "." will become
  /// "askap.ASKAP_PACKAGE_NAME.inname"
  /// @li If the inanme doesn't start with "." it will be under the askap logger,
  /// i.e. askap.inname. This is used by the ASKAP_LOGGER macro.

  /// @return a standardised logger package name
  inline std::string generateLoggerName(const std::string& inname)
  {
    const std::string name("askap.");
    if (inname.length() > 0) {
      if (inname[0] == '.') {
        return (name + std::string(ASKAP_PACKAGE_NAME) + inname);
      } else {
        return (name + inname);
      }
    }
    return (name + std::string(ASKAP_PACKAGE_NAME));
  }

/// Add a key/value pair to the Mapped Diagnostic Context. This is done per
/// thread. These can be printed in the Pattern using %X
#define ASKAPLOG_PUTCONTEXT(key,val) log4cxx::MDC::put(key, val)

/// Remove the key/value from the MDC.
#define ASKAPLOG_REMOVECONTEXT(key) log4cxx::MDC::remove(key)

/// Check if ASKAPLOG_INIT has been called to initialise the logger.
/// Evaluates to true if it has been initialised, otherwise false.
/// This uses the fact that the root logger initially has no appenders
/// configured, and the configuration we do adds appenders.
#define ASKAPLOG_ISCONFIGURED \
      (log4cxx::Logger::getRootLogger()->getAllAppenders().size() ? true : false)

/// Initialise a logger from a file. If none is specified or found, it uses the default settings
#define ASKAPLOG_INIT(filename)                                        \
  do {                                                                  \
    if (std::string(filename).length() == 0) {                          \
      log4cxx::BasicConfigurator::configure();                          \
      break;                                                            \
    }                                                                   \
    if (!strstr(filename, ".log_cfg")) {                                \
      throw(askap::AskapError("Logger configuration file needs suffix .log_cfg")); \
    } else {                                                            \
      std::fstream logis;                                               \
      logis.open(filename, std::ios::in);                               \
      if (logis.is_open()) {                                            \
        logis.close();                                                  \
        log4cxx::PropertyConfigurator::configure(log4cxx::File(filename)); \
        break;                                                          \
      }                                                                 \
    }                                                                   \
    log4cxx::BasicConfigurator::configure();                            \
  } while(0)


/// Create a logger handle with a specific logger name. This needs to be called in ".cc" before any logging occurs. The logger name is generated by generateLoggerName.
#define ASKAP_LOGGER(handle, name)                                     \
  static log4cxx::LoggerPtr handle = log4cxx::Logger::getLogger(askap::generateLoggerName(std::string(name)));


/// Macros to generate a log message from a string
/// @name Logging macros
// @{
/// debug
#define ASKAPLOG_DEBUG(logger,message) AskapLog(logger,Debug,message)
/// info
#define ASKAPLOG_INFO(logger,message) AskapLog(logger,Info,message)
/// warn
#define ASKAPLOG_WARN(logger,message) AskapLog(logger,Warn,message)
/// error
#define ASKAPLOG_ERROR(logger,message) AskapLog(logger,Error,message)
/// fatal
#define ASKAPLOG_FATAL(logger,message) AskapLog(logger,Fatal,message)
// @}

/// Macros to generate a log message from a stream
/// @name Logging stream macros
// @{
/// debug
#define ASKAPLOG_DEBUG_STR(logger,stream) AskapLogStr(logger,Debug,stream)
/// info
#define ASKAPLOG_INFO_STR(logger,stream) AskapLogStr(logger,Info,stream)
/// warn
#define ASKAPLOG_WARN_STR(logger,stream) AskapLogStr(logger,Warn,stream)
/// error
#define ASKAPLOG_ERROR_STR(logger,stream) AskapLogStr(logger,Error,stream)
/// fatal
#define ASKAPLOG_FATAL_STR(logger,stream) AskapLogStr(logger,Fatal,stream)
// @}

/// Do the actual logging
#define AskapLog(logger,type,message)                                  \
  do {                                                                  \
    if (logger->is##type##Enabled()) {                                  \
      logger->forcedLog(log4cxx::Level::get##type(), message,           \
                        LOG4CXX_LOCATION);                              \
    }                                                                   \
  } while(0)

/// Do the actual logging of a stream
#define AskapLogStr(logger,type,stream)                                \
  do {                                                                  \
    if (logger->is##type##Enabled()) {                                  \
      std::ostringstream askap_log_oss;                                \
      askap_log_oss << stream;                                         \
      logger->forcedLog(log4cxx::Level::get##type(), askap_log_oss.str(), \
                        LOG4CXX_LOCATION);                              \
    }                                                                   \
  } while(0)

} // end namespaces

#endif
