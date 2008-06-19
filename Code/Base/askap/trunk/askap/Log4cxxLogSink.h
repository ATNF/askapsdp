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

#ifndef ASKAP_LOG4CXXLOGSINK_H
#define ASKAP_LOG4CXXLOGSINK_H

//#Includes
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogFilter.h>
#include <stdexcept>

namespace askap {

  /// @brief log4cxx LogSink for Casa log messages
  /// @details
  /// This class can be used to redirect the casacore log messages to
  /// the log4cxx logging system used by askap.
  /// It can be achieved by defining this sink as the global casa LogSink
  /// as follows:
  /// @code
  ///   casa::LogSinkInterface* globalSink = new askap::Log4cxxLogSink;
  ///   casa::LogSink::globalSink (globalSink);
  /// @endcode
  /// Note that the pointer is taken over by LogSink, so the newly
  /// created Log4cxxLogSink object should NOT be deleted.
  class Log4cxxLogSink : public casa::LogSinkInterface
  {
  public:
    /// By default no filtering is done.
    Log4cxxLogSink();

    /// Create the sink with the given filter level.
    /// The default is that everything passes the filter.
    /// @{
    explicit Log4cxxLogSink (casa::LogMessage::Priority filter);
    explicit Log4cxxLogSink (const casa::LogFilterInterface& filter);
    /// @}

    ~Log4cxxLogSink();

    /// If the message passes the filter, write it to the log4cxx sink.
    virtual casa::Bool postLocally (const casa::LogMessage& message);

    /// Clear the local sink (i.e. remove all messages from it).
    virtual void clearLocally();

    /// Returns the id for this class...
    static casa::String localId();
    /// Returns the id of the LogSink in use...
    casa::String id() const;

  private:
    /// Copying is forbidden.
    /// @{
    Log4cxxLogSink (const Log4cxxLogSink& other);
    Log4cxxLogSink& operator= (const Log4cxxLogSink& other);
    /// @}
  };

} // end namespaces

#endif
