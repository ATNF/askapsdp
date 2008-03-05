//# tClusterDesc.cc: Test program for class ClusterDesc
//#
//# Copyright (C) 2007
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
