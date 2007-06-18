/// @file DataAccessError.h
/// @brief Exception classes used at the data access layer
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef DATA_ACCESS_ERROR_H
#define DATA_ACCESS_ERROR_H

/// std includes
#include <string>

/// own includes
#include <conrad/ConradError.h>

namespace conrad {

namespace synthesis {

/// @brief general exception class used in the data access layer
struct DataAccessError : public ConradError
{
  /// constructor - pass the message to the base class
  ///
  /// @param message a string message explaining what happens
  ///
  explicit DataAccessError(const std::string& message);
};

/// @brief exception class indicating a logic error in the data access layer
struct DataAccessLogicError : public DataAccessError
{
  /// constructor - pass the message to the base class
  ///
  /// @param message a string message explaining what happens
  ///
  explicit DataAccessLogicError(const std::string& message);
};

} // namespace conrad

} // namespace conrad

#endif // #ifndef DATA_ACCESS_ERROR_H
