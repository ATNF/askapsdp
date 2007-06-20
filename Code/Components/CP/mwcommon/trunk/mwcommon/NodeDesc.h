/// @file
/// @brief Description of a node in a cluster.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_NODEDESC_H
#define CONRAD_MWCOMMON_NODEDESC_H

//# Includes
#include <APS/ParameterSet.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace conrad { namespace cp {

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
