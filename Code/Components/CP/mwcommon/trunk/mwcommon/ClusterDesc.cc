//# ClusterDesc.cc: Description of a cluster
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/ClusterDesc.h>

using namespace std;

namespace askap { namespace cp {

  ClusterDesc::ClusterDesc (const LOFAR::ACC::APS::ParameterSet& parset)
  {
    itsName = parset.getString ("ClusterName");
    int nnode = parset.getInt32 ("NNodes");
    for (int i=0; i<nnode; ++i) {
      ostringstream prefix;
      prefix << "Node" << i << '.';
      LOFAR::ACC::APS::ParameterSet subset = parset.makeSubset (prefix.str());
      NodeDesc node(subset);
      itsNodes.push_back (node);
      add2Map (node);
    }
  }

  void ClusterDesc::write (ostream& os) const
  { 
    os << "ClusterName = " << itsName << endl;
    os << "NNodes = " << itsNodes.size() << endl;
    for (unsigned i=0; i<itsNodes.size(); ++i) {
      ostringstream prefix;
      prefix << "Node" << i << '.';
      itsNodes[i].write (os, prefix.str());
    }
  }

  void ClusterDesc::addNode (const NodeDesc& node)
  {
    itsNodes.push_back (node);
    add2Map (node);
  }

  void ClusterDesc::add2Map (const NodeDesc& node)
  {
    for (vector<string>::const_iterator iter = node.getFileSys().begin();
	 iter != node.getFileSys().end();
	 ++iter) {
      vector<string>& vec = itsFS2Nodes[*iter];
      vec.push_back (node.getName());
    }
  }

//   string ClusterDesc::findNode (const string& fileSystem,
// 				const map<string,int>& done) const
//   {
//     map<string,vector<string> >::const_iterator iter =
//                                              itsFS2Nodes.find(fileSystem);
//     if (iter == itsFS2Nodes.end()) {
//       return "";
//     }
//     const vector<string>& nodes = iter->second;
//     for (unsigned i=0; i<nodes.size(); ++i) {
//       if (done.find(nodes[i]) == done.end()) {
// 	return nodes[i];
//       }
//     }
//     return "";
//   }

}} // end namespaces
