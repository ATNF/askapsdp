/// @file
/// @brief Base visitor class to visit an MWSpec hierarchy.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCONTROL_MWSPECVISITOR_H
#define ASKAP_MWCONTROL_MWSPECVISITOR_H


namespace askap { namespace cp {

  //# Forward Declarations
  class MWMultiSpec;
  class MWSolveSpec;
  class MWCorrectSpec;
  class MWSubtractSpec;
  class MWPredictSpec;


  /// @ingroup mwcontrol
  /// @brief Base visitor class to visit an MWSpec hierarchy.

  /// This is a class to traverse a MWSpec composite using the visitor
  /// pattern (see Design Patterns, Gamma et al, 1995).
  /// It is the base class for all visitor classes.
  ///
  /// For each step in the composite, a visitXXX function is called where
  /// XXX is the step type. In this way many different visitors can be
  /// used without the need of implementing such functions in the MWSpec
  /// classes. The downside is that a visitYYY function needs to be added
  /// to all visitor classes if an new step type YYY is created.

  class MWSpecVisitor
  {
  public:
    /// Destructor.
    virtual ~MWSpecVisitor();

    /// Visit a \a visitMulti object. It traverses the object and visits its
    /// components.
    virtual void visitMulti (const MWMultiSpec&);

    /// Visit the various MWSpec types.
    /// The default implementations throw an MWError exception telling that
    /// the operation is not implemented in a derived class. So they need
    /// to be implemented in a derived Visitor class if such an object is
    /// expected to be used.
    /// @{
    virtual void visitSolve    (const MWSolveSpec&);
    virtual void visitCorrect  (const MWCorrectSpec&);
    virtual void visitSubtract (const MWSubtractSpec&);
    virtual void visitPredict  (const MWPredictSpec&);
    /// @}
  };
  
}} /// end namespaces

#endif
