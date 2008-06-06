/// @file
/// @brief Description of a node in a cluster.
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

#ifndef ASKAP_MWCOMMON_NODEDESC_H
#define ASKAP_MWCOMMON_NODEDESC_H

//# Includes
#include <APS/ParameterSet.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Description of a node in a cluster.

  /// This class holds the basic description of a node.
  /// It tells the name of the node and which file systems it has access to.
  ///
  /// Currently the information is made persistent in a LOFAR .parset file.
  /// In the future it needs to use the Centrol Processor Resource Manager.

  class NodeDesc
  {
  public:
    /// Construct an empty object.
    NodeDesc()
      {}

    /// Construct from the given parameterset.
    explicit NodeDesc (const LOFAR::ACC::APS::ParameterSet&);

    /// Set node name.
    void setName (const std::string& name)
      { itsName = name; }

    /// Add a file system it has access to.
    void addFileSys (const std::string& fsName)
      { itsFileSys.push_back (fsName); }

    /// Write it in parset format.
    void write (std::ostream& os, const std::string& prefix) const;

    /// Get the name.
    const std::string& getName() const
      { return itsName; }

    /// Get the file systems it has access to.
    const std::vector<std::string>& getFileSys() const
      { return itsFileSys; }

  private:
    std::string itsName;                    //# full name of the node
    std::vector<std::string> itsFileSys;    //# name of file systems
  };
    
}} /// end namespaces

#endif
