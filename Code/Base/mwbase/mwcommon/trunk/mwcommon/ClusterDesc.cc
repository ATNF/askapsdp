//# ClusterDesc.cc: Description of a cluster
//#
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
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/ClusterDesc.h>

using namespace std;

namespace askap { namespace mwbase {

  ClusterDesc::ClusterDesc (const LOFAR::ParameterSet& parset)
  {
    itsName = parset.getString ("ClusterName");
    int nnode = parset.getInt32 ("NNodes");
    for (int i=0; i<nnode; ++i) {
      ostringstream prefix;
      prefix << "Node" << i << '.';
      LOFAR::ParameterSet subset = parset.makeSubset (prefix.str());
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
