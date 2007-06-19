//# tClusterDesc.cc: Test program for class ClusterDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <mwcommon/ClusterDesc.h>
#include <mwcommon/ConradError.h>
#include <ostream>
#include <fstream>

using namespace conrad::cp;
using namespace std;

void check (const ClusterDesc& cl)
{
  CONRADASSERT (cl.getName() == "cl");
  CONRADASSERT (cl.getNodes().size() == 2);
  const vector<NodeDesc>& nodes = cl.getNodes();
  CONRADASSERT (nodes[0].getFileSys().size() == 2);
  CONRADASSERT (nodes[0].getFileSys()[0] == "fs0");
  CONRADASSERT (nodes[0].getFileSys()[1] == "fs1");
  CONRADASSERT (nodes[1].getFileSys().size() == 2);
  CONRADASSERT (nodes[1].getFileSys()[0] == "fs1");
  CONRADASSERT (nodes[1].getFileSys()[1] == "fs2");
  CONRADASSERT (cl.getMap().size() == 3);
  map<string,vector<string> >::const_iterator fsmap;
  fsmap = cl.getMap().find("fs0");
  CONRADASSERT (fsmap->second.size() == 1);
  CONRADASSERT (fsmap->second[0] == "node1");
  fsmap = cl.getMap().find("fs1");
  CONRADASSERT (fsmap->second.size() == 2);
  CONRADASSERT (fsmap->second[0] == "node1");
  CONRADASSERT (fsmap->second[1] == "node2");
  fsmap = cl.getMap().find("fs2");
  CONRADASSERT (fsmap->second.size() == 1);
  CONRADASSERT (fsmap->second[0] == "node2");
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
