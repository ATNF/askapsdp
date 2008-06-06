/// @file
/// @brief Specification of a BBS strategy
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCONTROL_MWSTRATEGYSPEC_H
#define ASKAP_MWCONTROL_MWSTRATEGYSPEC_H

//# Includes
#include <mwcontrol/MWMultiSpec.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace askap { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Specification of a BBS strategy

  /// This class contains the specification of a BBS strategy.
  /// It consists of two parts:
  /// <ul>
  ///  <li> The work domain specification which defines the work domain size and
  ///       optionally the basic data selection and integration.
  ///  <li> The name of the MWMultiSpec object containing the steps to be performed
  ///       when processing the data for this strategy.
  /// </ul>
  /// The strategy specification is read from a LOFAR .parset file.

  class MWStrategySpec
  {
  public:
    /// Default constructor.
    MWStrategySpec()
      {}

    /// Construct a MWSolveSpec having the name \a name. Configuration
    /// information for this step can be retrieved from the parameter set \a
    /// parset, by searching for keys <tt>Strategy.\a name</tt>.
    MWStrategySpec (const std::string& name,
		    const LOFAR::ACC::APS::ParameterSet& parset);

    /// Print the contents in human readable form into the output stream.
    void print (std::ostream& os) const;

    /// Return the step (possibly multi step) specification in this strategy.
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
