//# WorkersDesc.cc: Description of a workers
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/WorkersDesc.h>
#include <algorithm>

using namespace std;

namespace conrad { namespace cp {

  WorkersDesc::WorkersDesc (const ClusterDesc& cd)
    : itsFS2Nodes (cd.getMap())
  {
    // Reserve for 128 workers; might get more.
    itsLoad.reserve (128);
  }

  void WorkersDesc::addWorker (unsigned workerId, const string& nodeName,
			       const vector<int>& workTypes)
  {
    // Resize load vector if needed.
    // Initialize load for this worker.
    if (workerId >= itsLoad.size()) {
      itsLoad.resize (workerId+1);
    }
    itsLoad[workerId] = 0;
    for (vector<int>::const_iterator iter=workTypes.begin();
	 iter != workTypes.end();
	 ++iter) {
      MapN2W& mnw = itsMap[*iter];
      vector<unsigned>& vec = mnw[nodeName];
      vec.push_back (workerId);
    }
  }

  int WorkersDesc::findWorker (int workType, const string& fileSystem) const
  {
    // Find the worker type.
    map<int,MapN2W>::const_iterator workMap = itsMap.find(workType);
    if (workMap == itsMap.end()) {
      return -1;
    }
    // The worker has to operate on the given file system, so only nodes
    // with access to it will be considered.
    // Note that there is also an entry with an empty FS in case a worker
    // does not need a specific FS.
    if (fileSystem.empty()) {
      return findLowest (workMap->second);
    }
    return findLowest (workMap->second, fileSystem);
  }

  int WorkersDesc::findLowest (const MapN2W& workMap) const
  {
    // Find worker with lowest load.
    int worker = -1;
    int load   = 1000000;
    // Loop over all nodes and find worker with lowest load.
    for (MapN2W::const_iterator workers = workMap.begin();
	 workers != workMap.end();
	 ++workers) {
      const vector<unsigned>& workTypes = workers->second;
      for (vector<unsigned>::const_iterator witer=workTypes.begin();
	   witer != workTypes.end();
	   ++witer) {
	// We can stop if a worker with load 0 is found.
	if (itsLoad[*witer] < load) {
	  worker = *witer;
	  load   = itsLoad[*witer];
	  if (load == 0) break;
	}
      }
      if (load == 0) break;
    }
    return worker;
  }

  int WorkersDesc::findLowest (const MapN2W& workMap,
			       const string& fileSystem) const
  {
    // Find worker with lowest load.
    int worker = -1;
    int load   = 1000000;
    // Get all nodes with access to the file system.
    MapF2N::const_iterator nodes = itsFS2Nodes.find(fileSystem);
    if (nodes == itsFS2Nodes.end()) {
      return -1;
    }
    const vector<string>& nodesVec = nodes->second;
    for (vector<string>::const_iterator iter=nodesVec.begin();
	 iter != nodesVec.end();
	 ++iter) {
      // Loop over all nodes and find worker with lowest load.
      // We can stop if a worker with load 0 is found.
      MapN2W::const_iterator workers = workMap.find(*iter);
      if (workers != workMap.end()) {
	const vector<unsigned>& workTypes = workers->second;
	for (vector<unsigned>::const_iterator witer=workTypes.begin();
	     witer != workTypes.end();
	     ++witer) {
	  if (itsLoad[*witer] < load) {
	    worker = *witer;
	    load   = itsLoad[*witer];
	    if (load == 0) break;
	  }
	}
	if (load == 0) break;
      }
    }
    return worker;
  }

}} // end namespaces
