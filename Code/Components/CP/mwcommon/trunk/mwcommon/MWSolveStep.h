/// @file
/// @brief Base class for a step to process an MW solve command.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWSOLVESTEP_H
#define CONRAD_MWCOMMON_MWSOLVESTEP_H

#include <mwcommon/MWStep.h>

namespace conrad { namespace cp {

  /// @ingroup mwcommon
  /// @brief Base class for a step to process an MW solve command.

  /// This class defines a base class for step types that can solve
  /// for parameters by comparing a parameterized model to observed data.
  /// A derived class has to implement the detailed solve command.
  ///
  /// A solve step uses a prediffer and a solver exchanging messages.
  /// All MWSolveStep classes have in common that they are executed by the
  /// MasterControl in the same way, so all such classes have to obey the
  /// same communication protocol.
  /// This is:
  /// <ul>
  ///  <li> The step object is sent to all workers. The prediffers send a reply
  ///       which is forwarded to the solver.
  ///  <li> The prediffers get a getEq command and send a reply with e.g.
  ///       the normalized equations. They are forwarded to the solver.
  ///  <li> The solver gets a Solve command to solve the equations and send
  ///       a reply with the solution. This is forwarded to all prediffers.
  ///  <li> MasterControl tests if the reply from the solver says it has
  ///       converged. This flag must be the first (bool) value in the message
  ///       data. If not converged, step 2 and 3 are repeated.
  /// </ul>
  ///
  /// In fact, any step that has such an iterative character can be derived
  /// from this class. Examples are calibration and deconvolution. But also
  /// a distributed algorithm to find sources in an image cube can use it.

  class MWSolveStep: public MWStep
  {
  public:
    MWSolveStep()
    {}

    virtual ~MWSolveStep();
  };

}} /// end namespaces

#endif
