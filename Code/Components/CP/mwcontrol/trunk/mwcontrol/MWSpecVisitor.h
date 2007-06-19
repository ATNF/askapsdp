//# MWSpecVisitor.h: Base visitor class to visit an MWSpec hierarchy
//#
//# Copyright (C) 2007
//#
//# $Id$

#ifndef CONRAD_MWCONTROL_MWSPECVISITOR_H
#define CONRAD_MWCONTROL_MWSPECVISITOR_H

/// \file
/// Base component class of the MWSpec composite pattern.

//# Includes

namespace conrad { namespace cp {

  //# Forward Declarations
  class MWMultiSpec;
  class MWSolveSpec;
  class MWCorrectSpec;
  class MWSubtractSpec;
  class MWPredictSpec;


  /// This is a class to traverse a MWSpec composite using the visitor
  /// pattern (see Gamma, 1995). It is the base class for all visitor classes.

  class MWSpecVisitor
  {
  public:
    /// Destructor.
    virtual ~MWSpecVisitor();

    /// Visit a \a visitMulti object. It traverses the object and visits its
    /// components.
    virtual void visitMulti (const MWMultiSpec&);

    /// Visit the various MWSpec types.
    /// The default implementations throw a MWError exception telling that
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
