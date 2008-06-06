//# tClusterDesc.cc: Test program for class ClusterDesc
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
//#
//# $Id$

#include <mwcommon/ClusterDesc.h>
#include <mwcommon/AskapError.h>
#include <ostream>
#include <fstream>

using namespace askap::cp;
using namespace std;

void check (const ClusterDesc& cl)
{
  ASKAPASSERT (cl.getName() == "cl");
  ASKAPASSERT (cl.getNodes().size() == 2);
  const vector<NodeDesc>& nodes = cl.getNodes();
  ASKAPASSERT (nodes[0].getFileSys().size() == 2);
  ASKAPASSERT (nodes[0].getFileSys()[0] == "fs0");
  ASKAPASSERT (nodes[0].getFileSys()[1] == "fs1");
  ASKAPASSERT (nodes[1].getFileSys().size() == 2);
  ASKAPASSERT (nodes[1].getFileSys()[0] == "fs1");
  ASKAPASSERT (nodes[1].getFileSys()[1] == "fs2");
  ASKAPASSERT (cl.getMap().size() == 3);
  map<string,vector<string> >::const_iterator fsmap;
  fsmap = cl.getMap().find("fs0");
  ASKAPASSERT (fsmap->second.size() == 1);
  ASKAPASSERT (fsmap->second[0] == "node1");
  fsmap = cl.getMap().find("fs1");
  ASKAPASSERT (fsmap->second.size() == 2);
  ASKAPASSERT (fsmap->second[0] == "node1");
  ASKAPASSERT (fsmap->second[1] == "node2");
  fsmap = cl.getMap().find("fs2");
  ASKAPASSERT (fsmap->second.size() == 1);
  ASKAPASSERT (fsmap->second[0] == "node2");
}

void doIt()
{
  ClusterDesc cl;
  cl.setName ("cl");
  NodeDesc node1;
  node1.setName ("node1");
  node1.addFileSys ("fs0");
  node1.addFileSys ("fs1");
  cl.addNode (node1);
  NodeDesc node2;
  node2.setName ("node2");
  node2.addFileSys ("fs1");
  node2.addFileSys ("fs2");
  cl.addNode (node2);
  check(cl);
  // Write into parset file.
  ofstream fos("tClusterDesc_tmp.fil");
  cl.write (fos);
  // Read back.
  LOFAR::ACC::APS::ParameterSet parset("tClusterDesc_tmp.fil");
  ClusterDesc cl2(parset);
  check(cl2);
  cl = cl2;
  check(cl);
}

int main()
{
  try {
    doIt();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
