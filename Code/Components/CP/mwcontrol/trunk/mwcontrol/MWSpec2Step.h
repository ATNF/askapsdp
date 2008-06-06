/// @file
/// @brief Convert an MWSpec object to an MWStep object.
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

#ifndef ASKAP_MWCONTROL_MWSPEC2STEP_H
#define ASKAP_MWCONTROL_MWSPEC2STEP_H

//# Includes
#include <mwcontrol/MWSpecVisitor.h>
#include <mwcommon/MWMultiStep.h>


namespace askap { namespace cp {

  //# Forward Declarations
  class MWSingleSpec;
  class MWStepBBS;
  class MWStrategySpec;
  class MWStepBBSProp;
  class WorkDomainSpec;

  /// @ingroup mwcontrol
  /// @brief Convert an MWSpec object to an MWStep object.

  /// This MWSpec2Step visitor class converts the various MWSpec objects
  /// to MWStep objects and collects them in a single MWMultiStep. Thus if the
  /// MWSpec has multiple levels of MWMultiSpec objects, it is flattened to a
  /// single MWMultiStep.
  ///
  /// The class also has a (static) method to convert an MWStrategySpec object
  /// to an WorkDomainSpec object.

  class MWSpec2Step: public MWSpecVisitor
  {
  public:
    /// Destructor.
    virtual ~MWSpec2Step();

    /// Visit for the various MWSpec types.
    /// @{
    virtual void visitSolve    (const MWSolveSpec&);
    virtual void visitCorrect  (const MWCorrectSpec&);
    virtual void visitSubtract (const MWSubtractSpec&);
    virtual void visitPredict  (const MWPredictSpec&);
    /// @}

    /// Return all collected steps.
    const MWMultiStep& getSteps() const
      { return itsSteps; }

    /// Convert the strategy specification to a work domain specification.
    static WorkDomainSpec convertStrategy (const MWStrategySpec&);

  private:
    /// Set the common properties of each spec.
    void setProp (const MWSingleSpec& spec, MWStepBBSProp& step) const;

    MWMultiStep itsSteps;   /// collection of all steps
  };
  
}} /// end namespaces

#endif
