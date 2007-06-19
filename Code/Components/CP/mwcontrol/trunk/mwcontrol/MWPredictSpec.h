//# MWPredictSpec.h: Derived leaf class of the MWSpec composite pattern.
//#
//# Copyright (C) 2007
//#
//# $Id$

#ifndef CONRAD_MWCONTROL_MWPREDICTSPEC_H
#define CONRAD_MWCONTROL_MWPREDICTSPEC_H

/// \file
/// Derived leaf class of the MWSpec composite pattern.

//# Includes
#include <mwcontrol/MWSingleSpec.h>

namespace conrad { namespace cp {

  /// This is a so-called \e leaf class in the MWSpec composite pattern (see
  /// Gamma, 1995).
  /// \note Currently, a %MWPredictSpec is in fact identical to a
  /// MWSingleSpec. Only the classType() method is overridden.

  class MWPredictSpec : public MWSingleSpec
  {
  public:
    explicit MWPredictSpec(const MWSpec* parent = 0) : 
      MWSingleSpec(parent) {}

    MWPredictSpec(const std::string& name, 
		  const LOFAR::ACC::APS::ParameterSet& parSet,
		  const MWSpec* parent)
      : MWSingleSpec(name, parSet, parent) {}

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWSpecVisitor&) const;

    /// Print the contents of \c *this in human readable form into the output
    /// stream \a os.
    virtual void print (std::ostream& os, const std::string& indent) const;
  };

}} /// end namespaces

#endif
