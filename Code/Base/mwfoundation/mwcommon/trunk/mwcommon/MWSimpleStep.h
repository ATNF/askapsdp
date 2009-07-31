/// @file
/// @brief Base classes for simple MW commands (like subtract)
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

#ifndef ASKAP_MWCOMMON_MWSIMPLESTEP_H
#define ASKAP_MWCOMMON_MWSIMPLESTEP_H

#include <mwcommon/MWStep.h>

namespace askap { namespace mwbase {

  /// @ingroup mwcommon
  /// @brief Base class for a step to process a simple MW command.

  /// This class defines a class that serves as the base class for a
  /// simple MW step. A simple MW step is a step that can be executed
  /// directly by a worker without the need of interaction between workers.
  /// An example is a subtract or correct. A solve is not a simple step,
  /// because it requires interaction between workers.
  ///
  /// A derived MWStepVisitor classes can handle all simple step types
  /// in a single function.

  class MWSimpleStep: public MWStep
  {
  public:
    MWSimpleStep()
    {}

    virtual ~MWSimpleStep();

    /// Visit the object, so the visitor can process it.
    /// The default implementation uses the MWStepVisitor::visitsIMPLE
    /// function.
    virtual void visit (MWStepVisitor&) const;
  };



  /// @ingroup mwcommon
  /// @brief Base class for a step to process an MW subtract command.

  /// This class defines a step that subtracts a model from the data
  /// and writes the result into the VDS
  ///
  /// It uses the standard MWStep functionality (factory and visitor) to
  /// create and process the object.
  /// The object can be converted to/from blob, so it can be sent to workers.

  class MWSubtractStep: public MWSimpleStep
  {
  public:
    MWSubtractStep()
    {}

    virtual ~MWSubtractStep();

    /// Visit the object, so the visitor can process it.
    /// The default implementation uses the MWStepVisitor::visitSubtractct
    /// function.
    virtual void visit (MWStepVisitor&) const;
  };



  /// @ingroup mwcommon
  /// @brief Base class for a step to process an MW correct command.
  /// @brief Step to process the MW correct command.

  /// This class defines a step that corrects the data in the VDS for
  /// the given parameters.
  ///
  /// It uses the standard MWStep functionality (factory and visitor) to
  /// create and process the object.
  /// The object can be converted to/from blob, so it can be sent to workers.

  class MWCorrectStep: public MWSimpleStep
  {
  public:
    MWCorrectStep()
    {}

    virtual ~MWCorrectStep();

    /// Visit the object, so the visitor can process it.
    /// The default implementation uses the MWStepVisitor::visitCorrect
    /// function.
    virtual void visit (MWStepVisitor&) const;
  };



  /// @ingroup mwcommon
  /// @brief Base class for a step to process an MW predict command.

  /// This class defines a step that does a predict, i.e. that writes the
  /// predicted data set into the VDS.
  ///
  /// It uses the standard MWStep functionality (factory and visitor) to
  /// create and process the object.
  /// The object can be converted to/from blob, so it can be sent to workers.

  class MWPredictStep: public MWSimpleStep
  {
  public:
    MWPredictStep()
    {}

    virtual ~MWPredictStep();

    /// Visit the object, so the visitor can process it.
    /// The default implementation uses the MWStepVisitor::visitPredict
    /// function.
    virtual void visit (MWStepVisitor&) const;
  };


}} /// end namespaces

#endif
