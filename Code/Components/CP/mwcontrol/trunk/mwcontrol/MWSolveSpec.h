//# MWSolveSpec.h: The properties for solvable parameters
//#
//# Copyright (C) 2007
//#
//# $Id$

#ifndef CONRAD_MWCONTROL_MWSOLVESPEC_H
#define CONRAD_MWCONTROL_MWSOLVESPEC_H

/// \file
/// The properties for solvable parameters

//# Includes
#include <mwcontrol/MWSingleSpec.h>


namespace conrad { namespace cp {

  class MWSolveSpec : public MWSingleSpec
  {
  public:
    /// Default constructor. Construct an empty MWSolveSpec object and make
    /// it a child of the MWSpec object \a parent.
    explicit MWSolveSpec(const MWSpec* parent = 0);

    /// Construct a MWSolveSpec having the name \a name. Configuration
    /// information for this step can be retrieved from the parameter set \a
    /// parset, by searching for keys <tt>Step.\a name</tt>. \a parent
    /// is a pointer to the MWSpec object that is the parent of \c *this.
    MWSolveSpec(const std::string& name,
		const LOFAR::ACC::APS::ParameterSet& parset,
		const MWSpec* parent);

    virtual ~MWSolveSpec();

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWSpecVisitor&) const;

    /// Print the contents of \c *this in human readable form into the output
    /// stream \a os.
    virtual void print (std::ostream& os, const std::string& indent) const;

    /// @name Accessor methods
    /// @{
    unsigned getMaxIter() const
      { return itsMaxIter; }
    double getEpsilon() const
      { return itsEpsilon; }
    double getMinConverged() const
      { return itsMinConverged; }
    const std::vector<std::string>& getParms() const
      { return itsParms; }
    const std::vector<std::string>& getExclParms() const
      { return itsExclParms; }
    const DomainShape& getDomainShape() const
      { return itsDomainSize; }
    /// @}

  private:
    unsigned    itsMaxIter;          ///< Maximum number of iterations
    double      itsEpsilon;          ///< Convergence threshold
    double      itsMinConverged;     ///< Fraction that must have converged
    DomainShape itsDomainSize;       ///< Solve domain size.
    std::vector<std::string> itsParms;    ///< Names of the solvable parameters
    std::vector<std::string> itsExclParms;///< Parameters to be excluded
  };

}} /// end namespaces

#endif
