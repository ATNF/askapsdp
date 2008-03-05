/// @file
/// @brief Convert an MWSpec object to an MWStep object.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
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
