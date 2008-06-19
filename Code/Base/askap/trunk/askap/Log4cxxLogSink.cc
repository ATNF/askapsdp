/// @file
/// @brief log4cxx LogSink for Casa log messages

/// @copyright (c) 2008 CSIRO
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
/// @author Ger van Diepen (gvd AT astron DOT nl)

#include <askap/Log4cxxLogSink.h>

#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".CASA");


namespace askap {

  Log4cxxLogSink::Log4cxxLogSink()
    : casa::LogSinkInterface (casa::LogFilter())
  {}

  Log4cxxLogSink::Log4cxxLogSink (casa::LogMessage::Priority filter)
    : casa::LogSinkInterface (casa::LogFilter(filter))
  {}

  Log4cxxLogSink::Log4cxxLogSink (const casa::LogFilterInterface& filter)
    : casa::LogSinkInterface (filter)
  {}

  Log4cxxLogSink::~Log4cxxLogSink()
  {}

  casa::Bool Log4cxxLogSink::postLocally (const casa::LogMessage& message)
  {
    casa::Bool posted = casa::False;
    if (filter().pass(message)) {
      std::string msg (message.origin().location() + ": " + message.message());
      posted = casa::True;
      switch (message.priority()) {
	case casa::LogMessage::DEBUGGING:
	  ///case casa::LogMessage::DEBUG2:
	  ///case casa::LogMessage::DEBUG1:
	{
	  ASKAPLOG_DEBUG (logger, msg);
	  break;
	}
	///case casa::LogMessage::NORMAL5:
	///case casa::LogMessage::NORMAL4:
	///case casa::LogMessage::NORMAL3:
	///case casa::LogMessage::NORMAL2:
	///case casa::LogMessage::NORMAL1:
	case casa::LogMessage::NORMAL:
	{
	  ASKAPLOG_INFO (logger, msg);
	  break;
	}
	case casa::LogMessage::WARN:
	{
	  ASKAPLOG_WARN (logger, msg);
	  break;
	}
	case casa::LogMessage::SEVERE:
	{
	  ASKAPLOG_ERROR (logger, msg);
	  break;
	}
      }
    }
    return posted;
  }

  void Log4cxxLogSink::clearLocally()
  {}

  casa::String Log4cxxLogSink::localId()
  {
    return casa::String("Log4cxxLogSink");
  }

  casa::String Log4cxxLogSink::id() const
  {
    return casa::String("Log4cxxLogSink");
  }

} // end namespaces
