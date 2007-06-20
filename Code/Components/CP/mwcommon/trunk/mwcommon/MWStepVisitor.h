/// @file
/// @brief Base visitor class to visit an MWStep hierarchy.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWSTEPVISITOR_H
#define CONRAD_MWCOMMON_MWSTEPVISITOR_H


namespace conrad { namespace cp {

  //# Forward Declarations
  class MWMultiStep;
  class MWSolveStep;
  class MWCorrectStep;
  class MWSubtractStep;
  class MWPredictStep;


  /// @ingroup mwcommon
  /// @brief Base visitor class to visit an MWStep hierarchy.

  /// This is a class to traverse a MWStep composite using the visitor
  /// pattern (see Design Patterns, Gamma et al, 1995).
  /// It is the base class for all visitor classes.
  ///
  /// For each step in the composite, a visitXXX function is called where
  /// XXX is the step type. In this way many different visitors can be
  /// used without the need of implementing such functions in the MWStep
  /// classes. The downside is that a visitYYY function needs to be added
  /// to all visitor classes if an new step type YYY is created.

  class MWStepVisitor
  {
  public:
    /// Destructor.
    virtual ~MWStepVisitor();

    /// Visit a \a MWMultiStep object. It traverses the object and visits its
    /// components.
    virtual void visitMulti (const MWMultiStep&);

    /// Visit the various MWStep types.
    /// The default implementations throw an MWError exception telling that
    /// the operation is not implemented in a derived class. So a visitXXX
    /// function only need to be implemented in a derived Visitor class if
    /// the step type XXX is expected to be used.
    /// @{
    virtual void visitSolve    (const MWSolveStep&);
    virtual void visitCorrect  (const MWCorrectStep&);
    virtual void visitSubtract (const MWSubtractStep&);
    virtual void visitPredict  (const MWPredictStep&);
    /// @}
  };
  
}} /// end namespaces

#endif
