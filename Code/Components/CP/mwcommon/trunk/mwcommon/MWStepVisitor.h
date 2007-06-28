/// @file
/// @brief Base visitor class to visit an MWStep hierarchy.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWSTEPVISITOR_H
#define CONRAD_MWCOMMON_MWSTEPVISITOR_H

#include <string>
#include <map>

namespace conrad { namespace cp {

  //# Forward Declarations
  class MWStep;
  class MWMultiStep;
  class MWSolveStep;
  class MWSimpleStep;
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
    /// Define the visit function for an arbitrary MWStep object.
    typedef void VisitFunc (MWStepVisitor&, const MWStep&);

    /// Destructor.
    virtual ~MWStepVisitor();

    /// Visit an \a MWMultiStep object. It traverses the object and visits its
    /// components.
    /// It is possible to override this function in a derived class, but
    /// that will hardly be necessary.
    virtual void visitMulti (const MWMultiStep&);

    /// Visit for an \a MWSolveStep type.
    /// The default implementation calls the \a visit function for an
    /// arbitrary \a MWStep object.
    virtual void visitSolve    (const MWSolveStep&);

    /// Visit a more specialized \a MWSimpleStep type.
    /// The default implementations call the \a visitSimple function.
    /// @{
    virtual void visitSubtract (const MWSubtractStep&);
    virtual void visitCorrect  (const MWCorrectStep&);
    virtual void visitPredict  (const MWPredictStep&);
    /// @}

    /// Visit for an arbitrary \a MWSimpleStep type.
    /// The default implementation calls the \a visit function for an
    /// arbitrary \a MWStep object.
    virtual void visitSimple (const MWSimpleStep&);

    /// Visit for an arbitrary \a MWStep type.
    /// The default implementation calls the \a VisitFunc function which
    /// is registered for the type name of the \a MWStep object.
    /// If not registered, it calls visitStep.
    void visit (const MWStep&);

    /// Visit for an arbitrary \a MWStep type.
    /// The default implementation throws an exception that the step cannot
    /// be handled.
    virtual void visitStep (const MWStep&);

    /// Register a visit function for an MWStep with the given name.
    /// This can be used for other types of MWStep objects.
    /// The given function will usually be a static function in a derived
    /// visitor class calling a class member function. It can look like:
    /// <pre>
    ///   void MyVisitor::doXXX (MWStepVisitor& visitor, const MWStep& step)
    ///     { dynamic_cast<MyVisitor&>(visitor).visitXXX(
    ///                     dynamic_cast<const MWSTepXXX&>(step)); }
    /// </pre>
    /// The casts are kind of ugly, but unavoidable.
    /// The doXXX functions can be registered by the constructor.
    void registerVisit (const std::string& name, VisitFunc*);

  private:
    std::map<std::string, VisitFunc*> itsMap;
  };
  
}} /// end namespaces

#endif
