//# MWMultiSpec.h: Derived composite class of the MWSpec composite pattern.
//#
//# Copyright (C) 2007
//#
//# $Id$

#ifndef CONRAD_MWCONTROL_MWMULTISPEC_H
#define CONRAD_MWCONTROL_MWMULTISPEC_H

/// \file
/// Derived composite class of the MWSpec composite pattern.

//# Includes
#include <mwcontrol/MWSpec.h>
#include <list>

namespace conrad { namespace cp {

  /// This is the so-called \e composite class in the composite pattern (see
  /// Gamma, 1995). The composite class contains pointers to zero or more
  /// MWSpec (component) objects.

  class MWMultiSpec : public MWSpec
  {
  public:
    /// Construct a MWMultiSpec. \a name identifies the step name in the
    /// parameter set file. It does \e not uniquely identify the step \e
    /// object being created. The third argument is used to pass a
    /// backreference to the parent MWSpec object.
    MWMultiSpec(const std::string& name,
		const LOFAR::ACC::APS::ParameterSet& parset,
		const MWSpec* parent);

    /// Default constructor. Construct an empty MWMultiSpec object and make
    /// it a child of the MWSpec object \a parent.
    explicit MWMultiSpec(const MWSpec* parent = 0) : MWSpec(parent) {}

    virtual ~MWMultiSpec();

    /// Add a step.
    void push_back (const MWSpec::ShPtr& spec)
      { itsSpecs.push_back (spec); }

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWSpecVisitor&) const;

    /// Print the contents of \c *this in human readable form into the output
    /// stream \a os.
    virtual void print (std::ostream& os, const std::string& indent) const;

    /// Define functions and so to iterate.
    /// @{
    typedef std::list<MWSpec::ShPtr>::const_iterator const_iterator;
    const_iterator begin() const
      { return itsSpecs.begin(); }
    const_iterator end() const
      { return itsSpecs.end(); }
    /// @}

  private:
    /// Check to see if there's an infinite recursion present in the
    /// definition of a MWMultiSpec. This can happen when one of the specs
    /// (identified by the argument \a name) defining the MWMultiSpec refers
    /// directly or indirectly to that same MWMultiSpec. 
    void infiniteRecursionCheck (const std::string& name) const;

    /// Vector holding a sequence of MWSpecs.
    std::list<MWSpec::ShPtr> itsSpecs;
  };

  /// @}
    
}} /// end namespaces

#endif
