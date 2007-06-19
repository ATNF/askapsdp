//# MWStrategySpec.h: The specification of a strategy
//#
//# Copyright (C) 2007

#ifndef CONRAD_MWCONTROL_MWSTRATEGYSPEC_H
#define CONRAD_MWCONTROL_MWSTRATEGYSPEC_H

/// \file
/// The properties for solvable parameters

//# Includes
#include <mwcontrol/MWMultiSpec.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace conrad { namespace cp {

  class MWStrategySpec
  {
  public:
    /// Default constructor.
    MWStrategySpec()
      {}

    /// Construct a MWSolveSpec having the name \a name. Configuration
    /// information for this step can be retrieved from the parameter set \a
    /// parset, by searching for keys <tt>Step.\a name</tt>. \a parent
    /// is a pointer to the MWSpec object that is the parent of \c *this.
    MWStrategySpec (const std::string& name,
		    const LOFAR::ACC::APS::ParameterSet& parset);

    /// Print the contents of \c this into the output stream \a os.
    void print (std::ostream& os) const;

    /// Return the step (possibly multi step) in this strategy.
    const MWMultiSpec& getStep() const
      { return itsStep; }

    /// @name Accessor methods
    /// @{
    const std::string&              getName() const
      { return itsName; }
    const std::vector<std::string>& getStations() const
      { return itsStations; }
    const std::string&              getInputData() const
      { return itsInputData; }
    const std::vector<std::string>& getCorrType() const
      { return itsCorrType; }
    const std::string&              getCorrSelection() const
      { return itsCorrSelection; }
    const DomainShape&              getWorkDomainSize() const
      { return itsWorkDomainSize; }
    const DomainShape&              getIntegration() const
      { return itsIntegration; }
    /// @}

  private:
    /// The name of the strategy.
    std::string              itsName;

    /// Names of the stations to use. Names may contains wildcards, like \c *
    /// and \c ?. Expansion of wildcards will be done in the BBS kernel, so
    /// they will be passed unaltered by BBS control.
    std::vector<std::string> itsStations;

    /// Name of the MS input data column
    std::string              itsInputData;

    /// The work domain size
    DomainShape              itsWorkDomainSize;
      
    //// Correlation types. E.g., ["XX", "XY", "YX", "YY"].
    std::vector<std::string> itsCorrType;

    //// Antenna correlation types.
    //// Valid values: "NONE", "AUTO", "CROSS", "ALL"
    std::string              itsCorrSelection;

    /// Integration intervals in frequency (Hz) and time (s).
    DomainShape              itsIntegration;

    /// The step(s) in this strategy.
    MWMultiSpec              itsStep;
  };


  /// Write the contents of a BBSStrategy to an output stream.
  std::ostream& operator<< (std::ostream&, const MWStrategySpec&);

}} /// end namespaces

#endif
