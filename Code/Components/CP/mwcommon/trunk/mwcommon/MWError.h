/// @file
/// @brief Basic exception for master/worker related errors.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCOMMON_MWERROR_H
#define ASKAP_MWCOMMON_MWERROR_H

#include <askap/AskapError.h>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Basic exception for master/worker related errors.

  /// This class defines the basic MW exception.
  /// Only this basic exception is defined so far. In the future, some more 
  /// fine-grained exceptions might be derived from it.

  class MWError: public askap::AskapError
  {
  public:
    /// Create the exception object with the given message.
    explicit MWError (const std::string& message);

    virtual ~MWError() throw();
  };


}} /// end namespaces

#endif
