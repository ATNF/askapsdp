/// @file
/// @brief Base class for CONRAD exceptions
/// @author Ger van Diepen (gvd AT astron DOT nl)

#ifndef CONRAD_ERROR_H
#define CONRAD_ERROR_H

#include <string>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace conrad {

  /// Define the base conrad exception class.
  class ConradError: public std::runtime_error
  {
  public:
    /// Constructor taking a message
    explicit ConradError(const std::string& message);
    /// empty destructor
    virtual ~ConradError() throw();
  };

  /// Define the conrad check exception class.
  class CheckError: public ConradError
  {
  public:
    /// Constructor taking a message
    explicit CheckError(const std::string& message);
    /// empty destructor
    virtual ~CheckError() throw();
  };

  /// Define the conrad assert exception class.
  class AssertError: public ConradError
  {
  public:
    /// Constructor taking a message
    explicit AssertError(const std::string& message);
    /// empty destructor
    virtual ~AssertError() throw();
  };


  /// Macro to throw an exception where the exception text can be formatted.
  /// For example:
  /// CONRADTHROW(ConradError, "File " << fileName << " could not be opened");
  //  Note that a do/while is used to be able to use the macro everywhere.
#define CONRADTHROW(exc,messageStream)   \
    {					 \
      std::ostringstream conrad_log_oss; \
      conrad_log_oss << messageStream;	 \
      throw exc(conrad_log_oss.str());   \
    }

  /// Check a condition and throw an exception if it fails.
  /// It appends the confdition text to the message stream.
#define CONRADCHECK(condition,messageStream) \
    if (!(condition)) {			     \
      CONRADTHROW(CheckError, messageStream \
                   << " ('" << #condition << "' failed)");	\
    }

  /// Do an assert and throw an exception with the file and line if it fails.
#define CONRADASSERT(condition) \
    if (!(condition)) {							\
      CONRADTHROW(AssertError, #condition << " failed in " <<  __FILE__  \
		   << ':' << __LINE__);					  \
    }

  /// Do an assert only if in debug mode.
#ifdef CONRAD_DEBUG
#define CONRADDEBUGASSERT(condition) CONRADASSERT(condition)
#else
#define CONRADDEBUGASSERT(condition)
#endif


} // end namespaces

#endif
