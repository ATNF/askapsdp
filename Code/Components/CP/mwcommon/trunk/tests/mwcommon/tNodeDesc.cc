//# tNodeDesc.cc: Test program for class NodeDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <mwcommon/NodeDesc.h>
#include <mwcommon/ConradError.h>
#include <ostream>
#include <fstream>

using namespace conrad::cp;
using namespace std;

void check (const NodeDesc& node)
{
  CONRADASSERT (node.getName() == "node1");
  CONRADASSERT (node.getFileSys().size() == 2);
  CONRADASSERT (node.getFileSys()[0] == "fs0");
  CONRADASSERT (node.getFileSys()[1] == "fs1");
}

void doIt()
{
  NodeDesc node;
  node.setName ("node1");
  node.addFileSys ("fs0");
  node.addFileSys ("fs1");
  check(node);
  // Write into parset file.
  ofstream fos("tNodeDesc_tmp.fil");
  node.write (fos, "");
  // Read back.
  LOFAR::ACC::APS::ParameterSet parset("tNodeDesc_tmp.fil");
  NodeDesc node2(parset);
  check(node2);
  node = node2;
  check(node);
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
