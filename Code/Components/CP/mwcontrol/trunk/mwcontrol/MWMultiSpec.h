/// @file
/// @brief Specification of a step containing multiple other steps.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCONTROL_MWMULTISPEC_H
#define CONRAD_MWCONTROL_MWMULTISPEC_H

//# Includes
#include <mwcontrol/MWSpec.h>
#include <list>

namespace conrad { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Specification of a step containing multiple other steps.

  /// This is the so-called \e composite class in the composite pattern (see
  /// Design Patterns, Gamma et al, 1995). The composite class contains shared
  /// pointers to zero or more MWSpec (component) objects.
  /// This class is very useful to combine multiple steps which can be treated
  /// as a single step. They object is created from the contents of a LOFAR
  /// .parset file.
  ///
  /// The contained objects get their default values from the settings
  /// in this parent MWMultiSpec object.

  class MWMultiSpec : public MWSpec
  {
  public:
    /// Construct an empty MWMultiSpec object.
    MWMultiSpec();

    /// Construct a MWMultiSpec. \a name identifies the step name in the
    /// parameter set file. It does \e not uniquely identify the step \e
    /// object being created. The third argument is used to pass a
    /// backreference to the parent MWSpec object.
    MWMultiSpec(const std::string& name,
		const LOFAR::ACC::APS::ParameterSet& parset,
		const MWSpec* parent);

    virtual ~MWMultiSpec();

    /// Add a step.
    void push_back (const MWSpec::ShPtr& spec)
      { itsSpecs.push_back (spec); }

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWSpecVisitor&) const;

    /// Print the contents in human readable form into the output stream.
    /// Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;

    /// Define functions to do STL style iteration.
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

}} /// end namespaces

#endif
