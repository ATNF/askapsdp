/// @file
/// @brief A step consisting of several other steps.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWMULTISTEP_H
#define CONRAD_MWCOMMON_MWMULTISTEP_H

#include <mwcommon/MWStep.h>
#include <list>

namespace conrad { namespace cp {

  /// A step consisting of several other steps.

  /// This class makes it possible to form a list of MWStep objects.
  /// Note that the class itself is an MWStep, so the list can be nested.
  /// The \a visit function will call \a visit of each step in the list.
  ///
  /// It uses the standard MWStep functionality (factory and visitor) to
  /// create and process the object.
  /// The object can be converted to/from blob, so it can be sent to workers.

  class MWMultiStep: public MWStep
  {
  public:
    virtual ~MWMultiStep();

    /// Clone the step object.
    virtual MWMultiStep* clone() const;

    /// Create a new object of this type.
    static MWStep::ShPtr create();

    /// Register the create function in the MWStepFactory.
    static void registerCreate();

    /// Add a step object (in fact, a clone is added).
    void push_back (const MWStep&);

    /// Visit the object, which visits each step.
    virtual void visit (MWStepVisitor&) const;

    /// Convert to/from blob.
    /// Note that reading back from a blob uses MWStepFactory to
    /// create the correct objects.
    /// @{
    virtual void toBlob (LOFAR::BlobOStream&) const;
    virtual void fromBlob (LOFAR::BlobIStream&);
    /// @}

    /// Define functions and so to iterate in the STL way.
    /// @{
    typedef std::list<MWStep::ShPtr>::const_iterator const_iterator;
    const_iterator begin() const
      { return itsSteps.begin(); }
    const_iterator end() const
      { return itsSteps.end(); }
    /// @}

  private:
    std::list<MWStep::ShPtr> itsSteps;
  };

}} /// end namespaces

#endif
