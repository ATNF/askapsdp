/// @file
/// @brief Describe an entire visibility data set
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

#ifndef ASKAP_MWCOMMON_VDSDESC_H
#define ASKAP_MWCOMMON_VDSDESC_H

//# Includes
#include <mwcommon/VdsPartDesc.h>
#include <casa/Utilities/Regex.h>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Describe an entire visibility data set

  /// This class holds the description of an entire visibility data set (VDS).
  /// In VdsPartDesc objects it describes the parts it consists of and
  /// on which file systems they are located.
  /// A VdsPartDesc object is also used to describe the entire VDS.
  /// Furthermore it contains the names of all antennae, which can be used
  /// to map the antenna name to the antenna number when a selection on
  /// antenna names is done.
  ///
  /// Currently the information is made persistent in a LOFAR .parset file.
  /// In the future it needs to use the Centrol Processor Resource Manager.

  class VdsDesc
  {
  public:
    /// Construct with a description of the entire visibility data set.
    /// Also supply a vector mapping antenna name to number.
    VdsDesc (const VdsPartDesc&, const std::vector<std::string>& antNames);

    /// Construct from the given parameterset.
    /// @{
    explicit VdsDesc (const std::string& parsetName);
    explicit VdsDesc (const LOFAR::ACC::APS::ParameterSet& parset)
      { init (parset); }
    /// @}

    /// Add a part.
    void addPart (const VdsPartDesc& part)
      { itsParts.push_back (part); }

    /// Get the description of the given part.
    const VdsPartDesc& getPart (int part) const
      { return itsParts[part]; }

    /// Get the description of the VDS.
    const VdsPartDesc& getDesc() const
      { return itsDesc; }

    /// Get antennas names.
    const std::vector<std::string>& getAntNames() const
      { return itsAntNames; }

    /// Convert an antenna name to its index.
    /// -1 is returned if not found.
    int antNr (const std::string& name) const;

    /// Convert an antenna regex to indices.
    std::vector<int> antNrs (const casa::Regex& names) const;

    /// Write it in parset format.
    void write (std::ostream& os) const;

  private:
    /// Fill the object from the given parset file.
    void init (const LOFAR::ACC::APS::ParameterSet& parset);

    VdsPartDesc              itsDesc;
    std::vector<VdsPartDesc> itsParts;
    std::vector<std::string> itsAntNames;     //# maps antennanr to name
  };

}} /// end namespaces

#endif
