/// @file
/// @brief Description of all workers.
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

#ifndef ASKAP_MWCOMMON_WORKERSDESC_H
#define ASKAP_MWCOMMON_WORKERSDESC_H

//# Includes
#include <mwcommon/ClusterDesc.h>
#include <string>
#include <vector>

namespace askap { namespace mwbase {

  /// @ingroup mwcommon
  /// @brief Description of all workers.

  /// This class holds the description of the workers in an MW run.
  /// For each worker it describes on which node it runs and which types
  /// of work it can perform.
  /// Combined with the cluster description, this information is used
  /// to determine which worker can be used to perform a given type of work
  /// on data on a given file system. In determining this, it keeps track of
  /// the workload to avoid that the same worker is selected again and again.

  class WorkersDesc
  {
  public:
    /// Construct from the given cluster description.
    WorkersDesc (const ClusterDesc&);

    /// Add a worker with the given id.
    /// It can do the work types given in the vector on the given node.
    void addWorker (unsigned id, const std::string& node,
		    const std::vector<int>& workTypes);

    /// Increase or decrease the load for the given worker.
    /// Return the new load.
    /// @{
    int incrLoad (unsigned worker)
      { return ++itsLoad[worker]; }
    int decrLoad (unsigned worker)
      { return --itsLoad[worker]; }
    /// @}

    /// Find the worker with the lowest load that can perform the given
    /// work type for data on the given file system.
    /// The file system can be empty indicating that any worker can do it.
    /// It returns -1 if no suitable worker could be found.
    int findWorker (int workType, const std::string& fileSystem) const;

  private:
    /// Map giving the workers on each node.
    typedef std::map<std::string,std::vector<unsigned> > MapN2W;
    /// Map given the nodes with access to a file system.
    typedef std::map<std::string,std::vector<std::string> > MapF2N;

    /// Find worker with lowest load.
    /// @{
    int findLowest (const MapN2W& workMap) const;
    int findLowest (const MapN2W& workMap,
		    const std::string& fileSystem) const;
    /// @}

    MapF2N               itsFS2Nodes; //# map FileSystem to nodes
    std::map<int,MapN2W> itsMap;      //# map worktype to node/worker
    std::vector<int>     itsLoad;     //# load of each worker (#times used)
  };
    
}} /// end namespaces

#endif
