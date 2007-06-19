//# MWSpec2Step.h: Convert MWSpec objects to a MWMultiStep
//#
//# Copyright (C) 2007
//#
//# $Id$

#ifndef CONRAD_MWCONTROL_MWSPEC2STEP_H
#define CONRAD_MWCONTROL_MWSPEC2STEP_H

/// \file
/// Convert MWSpec objects to a MWMultiStep.

//# Includes
#include <mwcontrol/MWSpecVisitor.h>
#include <mwcommon/MWMultiStep.h>


namespace conrad { namespace cp {

  //# Forward Declarations
  class MWSingleSpec;
  class MWStepBBS;
  class MWStrategySpec;
  class WorkDomainSpec;


  /// This MWSpec visitor class converts the various MWSpec objects
  /// to MWStep objects and collects them in an MWMultiStep.
  /// It also has a (static) method to convert a strategy to WorkDomainSpec.

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
    /// Set the common fields of each spec.
    void setCommon (const MWSingleSpec& spec, MWStepBBS& step) const;

    MWMultiStep itsSteps;   /// collection of all steps
  };
  
}} /// end namespaces

#endif
