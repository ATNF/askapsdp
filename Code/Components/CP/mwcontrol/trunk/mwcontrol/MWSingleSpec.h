//# MWSingleSpec.h: Derived leaf class of the MWSpec composite pattern.
//#
//# Copyright (C) 2007
//#
//# $Id$

#ifndef CONRAD_MWCONTROL_MWSINGLESPEC_H
#define CONRAD_MWCONTROL_MWSINGLESPEC_H

/// \file
/// Derived leaf class of the MWSpec composite pattern.

//# Includes
#include <mwcontrol/MWSpec.h>

namespace conrad { namespace cp {

  /// This is a so-called \e leaf class in the MWSpec composite pattern (see
  /// Gamma, 1995).
  /// \note %MWSingleSpec not implemented as a leaf class; it contains a
  /// number of data members that are common to "real" MWSpec leaf classes,
  /// like MWSolveSpec.

  class MWSingleSpec : public MWSpec
  {
  public:
    virtual ~MWSingleSpec();

    /// Print the contents of \c *this in human readable form into the output
    /// stream \a os.
    void printSpec (std::ostream& os, const std::string& indent,
		    const std::string& type) const;

    /// Return the name of the data column to write data to.
    const std::string& getOutputData() const
      { return itsOutputData; }

  protected:
    /// Default constructor. Construct an empty MWSingleSpec object and make
    /// it a child of the MWSpec object \a parent.
    MWSingleSpec(const MWSpec* parent = 0);

    /// Construct a MWSingleSpec having the name \a name. Configuration
    /// information for this step can be retrieved from the parameter set \a
    /// parset, by searching for keys <tt>Step.\a name</tt>. \a parent
    /// is a pointer to the MWSpec object that is the parent of \c *this.
    MWSingleSpec(const std::string& name,
		 const LOFAR::ACC::APS::ParameterSet& parset,
		 const MWSpec* parent);

    /// Name of the data column to write data to.
    std::string itsOutputData;
  };

}} /// end namespaces

#endif
