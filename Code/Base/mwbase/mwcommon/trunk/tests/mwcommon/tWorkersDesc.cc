//# tWorkersDesc.cc: Test program for class WorkersDesc
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

#include <mwcommon/WorkersDesc.h>
#include <mwcommon/AskapError.h>
#include <ostream>

using namespace askap::cp;
using namespace std;

void doIt1()
{
  // First define the cluster.
  // File systems can be accessed from multiple nodes.
  ClusterDesc cl;
  cl.setName ("cl");
  NodeDesc node0;
  node0.setName ("node0");
  node0.addFileSys ("fs0");
  node0.addFileSys ("fs1");
  cl.addNode (node0);
  NodeDesc node1;
  node1.setName ("node1");
  node1.addFileSys ("fs1");
  node1.addFileSys ("fs2");
  cl.addNode (node1);
  NodeDesc node2;
  node2.setName ("node2");
  node2.addFileSys ("fs0");
  node2.addFileSys ("fs1");
  node2.addFileSys ("fs2");
  cl.addNode (node2);
  WorkersDesc wdesc(cl);
  // Now define all workers which can perform 2 work types.
  vector<int> wtypes(2);
  wtypes[0] = 0;
  wtypes[1] = 1;
  wdesc.addWorker (0, "node0", wtypes);
  wdesc.addWorker (1, "node1", wtypes);
  wdesc.addWorker (2, "node2", wtypes);
  // Now find a worker for a specific task on a file system.
  int worker;
  worker = wdesc.findWorker (0, "fs0");
  ASKAPASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  ASKAPASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs1");
  ASKAPASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  ASKAPASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs1");
  ASKAPASSERT (worker == 0);
  worker = wdesc.findWorker (0, "fs0");
  ASKAPASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs0");
  ASKAPASSERT (worker == 2);
  wdesc.incrLoad (worker);
  wdesc.incrLoad (0);
  wdesc.incrLoad (1);
  worker = wdesc.findWorker (1, "");
  ASKAPASSERT (worker == 2);
  wdesc.incrLoad (worker);
  ASKAPASSERT (wdesc.findWorker (2, "") == -1);
  ASKAPASSERT (wdesc.findWorker (0, "fs3") == -1);
}

void doIt2()
{
  // First define the cluster.
  // FIle systems can be accessed from a single node.
  ClusterDesc cl;
  cl.setName ("cl");
  NodeDesc node0;
  node0.setName ("node0");
  node0.addFileSys ("fs0");
  cl.addNode (node0);
  NodeDesc node1;
  node1.setName ("node1");
  node1.addFileSys ("fs1");
  cl.addNode (node1);
  NodeDesc node2;
  node2.setName ("node2");
  node2.addFileSys ("fs2");
  cl.addNode (node2);
  WorkersDesc wdesc(cl);
  // Now define all workers which can perform 2 work types.
  vector<int> wtypes(2);
  wtypes[0] = 0;
  wtypes[1] = 1;
  wdesc.addWorker (0, "node0", wtypes);
  wdesc.addWorker (1, "node1", wtypes);
  wdesc.addWorker (2, "node2", wtypes);
  // Now find a worker for a specific task on a file system.
  int worker;
  worker = wdesc.findWorker (0, "fs0");
  ASKAPASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs0");
  ASKAPASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  ASKAPASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs1");
  ASKAPASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  ASKAPASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  ASKAPASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  ASKAPASSERT (worker == 0);
  wdesc.incrLoad (worker);
  ASKAPASSERT (wdesc.findWorker (2, "") == -1);
  ASKAPASSERT (wdesc.findWorker (0, "fs4") == -1);
}

int main()
{
  try {
    doIt1();
    doIt2();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
