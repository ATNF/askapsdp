/// @file
/// @brief Description of a cluster and the nodes in it.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCOMMON_CLUSTERDESC_H
#define ASKAP_MWCOMMON_CLUSTERDESC_H

//# Includes
#include <mwcommon/NodeDesc.h>
#include <APS/ParameterSet.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Description of a cluster and the nodes in it.

  /// This class holds the basic description of a cluster.
  /// It defines which nodes are part of the cluster and which file systems
  /// each node has access to.
  /// If a data set is distributed over many file systems, the cluster
  /// description tells which node can handle a data set part on a particular
  /// file system.
  ///
  /// Currently the information is made persistent in a LOFAR .parset file.
  /// In the future it needs to use the Centrol Processor Resource Manager.

  class ClusterDesc
  {
  public:
    /// Construct an empty object.
    ClusterDesc()
      {}

    /// Construct from the given parameterset.
    explicit ClusterDesc (const LOFAR::ACC::APS::ParameterSet&);

    /// Set cluster name.
    void setName (const std::string& name)
      { itsName = name; }

    /// Add a file system it has access to.
    void addNode (const NodeDesc& node);

    /// Write it in parset format.
    void write (std::ostream& os) const;

    /// Get the name.
    const std::string& getName() const
      { return itsName; }

    /// Get all nodes.
    const std::vector<NodeDesc>& getNodes() const
      { return itsNodes; }

    /// Get the map of file system to node.
    const std::map<std::string, std::vector<std::string> >& getMap() const
      { return itsFS2Nodes; }

  private:
    /// Add entries to the mapping of FileSys to Nodes.
    void add2Map (const NodeDesc& node);

    std::string itsName;
    std::vector<NodeDesc> itsNodes;
    std::map<std::string, std::vector<std::string> > itsFS2Nodes;
  };
    
}} /// end namespaces

#endif
