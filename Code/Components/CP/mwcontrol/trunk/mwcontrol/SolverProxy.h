/// @file
/// @brief Base class for BBSKernel solver behaviour.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCONTROL_SOLVERPROXY_H
#define CONRAD_MWCONTROL_SOLVERPROXY_H

#include <mwcontrol/BBSProxy.h>


namespace conrad { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Base class for BBSKernel solver behaviour.

  /// This class is the base class for BBSKernel solver behaviour.
  /// A derived class needs to do the concrete implementation of the
  /// virtual BBSProxy functions.
  /// By registering the desired concrete class with type name "Solver"
  /// in the WorkerFactory, the MW framework will use that class as the
  /// solver proxy.
  ///
  /// In this way it is possible to use a simple test class (which can
  /// merely output the command on cout) to check the control flow.
  /// This is used by the test program \a tMWControl.

class SolverProxy: public BBSProxy
  {
  public:
    SolverProxy()
    {}

    virtual ~SolverProxy();

    /// Get the work types supported by the proxy (which is solve).
    virtual std::vector<int> getWorkTypes() const;
  };

}} /// end namespaces

#endif
