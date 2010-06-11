//# tNodeDesc.cc: Test program for class NodeDesc
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

#include <mwcommon/NodeDesc.h>
#include <mwcommon/AskapError.h>
#include <ostream>
#include <fstream>

using namespace askap::cp;
using namespace std;

void check (const NodeDesc& node)
{
  ASKAPASSERT (node.getName() == "node1");
  ASKAPASSERT (node.getFileSys().size() == 2);
  ASKAPASSERT (node.getFileSys()[0] == "fs0");
  ASKAPASSERT (node.getFileSys()[1] == "fs1");
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
  LOFAR::ParameterSet parset("tNodeDesc_tmp.fil");
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
