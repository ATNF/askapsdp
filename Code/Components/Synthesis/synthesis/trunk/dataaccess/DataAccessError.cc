/// @file DataAccessError.cc
/// @brief define exception classes used at the data access layer
/// @details this file contains implementations of the exception classes,
/// which can be thrown from the data access layer
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own includes
#include <dataaccess/DataAccessError.h>

using namespace conrad;
using namespace conrad::synthesis;

/// constructor - pass the message to the base class
///
/// @param message a string message explaining what happens
///
DataAccessError::DataAccessError(const std::string& message) :
     ConradError(message) {}

/// constructor - pass the message to the base class
///
/// @param message a string message explaining what happens
///
DataAccessLogicError::DataAccessLogicError(const std::string& message) :
     DataAccessError(message) {}
