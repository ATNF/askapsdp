/// @file
/// @brief Factory pattern to generate a WorkerProxy object.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_WORKERFACTORY_H
#define CONRAD_MWCOMMON_WORKERFACTORY_H

#include <mwcommon/WorkerProxy.h>
#include <map>
#include <string>

namespace conrad { namespace cp {

  /// @ingroup mwcommon
  /// @brief Factory pattern to generate a WorkerProxy object.

  /// This class contains a map of names to \a create functions
  /// of derived WorkerProxy objects. It is used to construct the correct
  /// WorkerProxy object given a type name.
  /// In this way one can choose which worker to use. For example, it makes
  /// it possible to use simple test workers to process prediffer and
  /// solver operations to check the control logic.

  class WorkerFactory
  {
  public:
    /// Define the signature of the function to create the worker.
    typedef WorkerProxy::ShPtr Creator ();

    /// Add a creator function.
    void push_back (const std::string& name, Creator*);
    
    /// Create the object of the given name.
    /// An exception is thrown if the name is not in the map.
    WorkerProxy::ShPtr create (const std::string& name) const;

  private:
    std::map<std::string, Creator*> itsMap;
  };


}} /// end namespaces

#endif
