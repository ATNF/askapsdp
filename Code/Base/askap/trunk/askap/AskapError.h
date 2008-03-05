/// @file
/// @brief Base class for ASKAP exceptions
/// @author Ger van Diepen (gvd AT astron DOT nl)

#ifndef ASKAP_ERROR_H
#define ASKAP_ERROR_H

#include <string>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace askap {

  /// Define the base askap exception class.
  class AskapError: public std::runtime_error
  {
  public:
    /// Constructor taking a message
    explicit AskapError(const std::string& message);
    /// empty destructor
    virtual ~AskapError() throw();
  };

  /// Define the askap check exception class.
  class CheckError: public AskapError
  {
  public:
    /// Constructor taking a message
    explicit CheckError(const std::string& message);
    /// empty destructor
    virtual ~CheckError() throw();
  };

  /// Define the askap assert exception class.
  class AssertError: public AskapError
  {
  public:
    /// Constructor taking a message
    explicit AssertError(const std::string& message);
    /// empty destructor
    virtual ~AssertError() throw();
  };


  /// Macro to throw an exception where the exception text can be formatted.
  /// For example:
  /// ASKAPTHROW(AskapError, "File " << fileName << " could not be opened");
  //  Note that a do/while is used to be able to use the macro everywhere.
#define ASKAPTHROW(exc,messageStream)   \
    {					 \
      std::ostringstream askap_log_oss; \
      askap_log_oss << messageStream <<" (thrown in "<<__FILE__  \
		   << ':' << __LINE__<<") ";	 \
      throw exc(askap_log_oss.str());   \
    }

  /// Check a condition and throw an exception if it fails.
  /// It appends the confdition text to the message stream.
#define ASKAPCHECK(condition,messageStream) \
    if (!(condition)) {			     \
      ASKAPTHROW(CheckError, messageStream \
                   << " ('" << #condition << "' failed)");	\
    }

  /// Do an assert and throw an exception with the file and line if it fails.
#define ASKAPASSERT(condition) \
    if (!(condition)) {	\
      ASKAPTHROW(AssertError, #condition << " failed");  \
    }

  /// Do an assert only if in debug mode.
#ifdef ASKAP_DEBUG
#define ASKAPDEBUGASSERT(condition) ASKAPASSERT(condition)
#else
#define ASKAPDEBUGASSERT(condition)
#endif


} // end namespaces

#endif
