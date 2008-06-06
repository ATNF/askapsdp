/// @file
/// @brief Base class for a step to process an MW solve command.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCOMMON_MWSOLVESTEP_H
#define ASKAP_MWCOMMON_MWSOLVESTEP_H

#include <mwcommon/MWStep.h>

namespace askap { namespace cp {

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

    /// Visit the object, so the visitor can process it.
    /// The default implementation uses the MWStepVisitor::visitSolve function.
    virtual void visit (MWStepVisitor&) const;
  };

}} /// end namespaces

#endif
