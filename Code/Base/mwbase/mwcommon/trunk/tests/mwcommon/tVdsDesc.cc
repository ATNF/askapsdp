//# tVdsDesc.cc: Test program for class VdsDesc
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

#include <mwcommon/VdsDesc.h>
#include <mwcommon/AskapError.h>
#include <ostream>
#include <fstream>

using namespace askap::cp;
using namespace casa;
using namespace std;

void checkVds (const VdsPartDesc& vds, double endTime)
{
  ASKAPASSERT (vds.getName() == "/usr/local/xyx");
  ASKAPASSERT (vds.getFileSys() == "node1:/usr");
  ASKAPASSERT (vds.getStartTime() == 0);
  ASKAPASSERT (vds.getEndTime() == endTime);
  ASKAPASSERT (vds.getNChan().size() == 2);
  ASKAPASSERT (vds.getNChan()[0] == 64);
  ASKAPASSERT (vds.getNChan()[1] == 128);
  ASKAPASSERT (vds.getStartFreqs().size() == 2);
  ASKAPASSERT (vds.getStartFreqs()[0] == 20);
  ASKAPASSERT (vds.getStartFreqs()[1] == 120);
  ASKAPASSERT (vds.getEndFreqs().size() == 2);
  ASKAPASSERT (vds.getEndFreqs()[0] == 100);
  ASKAPASSERT (vds.getEndFreqs()[1] == 300);
  ASKAPASSERT (vds.getAnt1().size() == 3);
  ASKAPASSERT (vds.getAnt1()[0] == 0);
  ASKAPASSERT (vds.getAnt1()[1] == 1);
  ASKAPASSERT (vds.getAnt1()[2] == 2);
  ASKAPASSERT (vds.getAnt2().size() == 3);
  ASKAPASSERT (vds.getAnt2()[0] == 0);
  ASKAPASSERT (vds.getAnt2()[1] == 1);
  ASKAPASSERT (vds.getAnt2()[2] == 3);
}

void check (const VdsDesc& vfds)
{
  checkVds (vfds.getDesc(), 1);
  checkVds (vfds.getPart(0), 2);
  ASKAPASSERT (vfds.getAntNames().size() == 4);
  ASKAPASSERT (vfds.getAntNames()[0] == "RT0");
  ASKAPASSERT (vfds.getAntNames()[1] == "RT1");
  ASKAPASSERT (vfds.getAntNames()[2] == "RT2");
  ASKAPASSERT (vfds.getAntNames()[3] == "RT3");
}

void tryAnt (const VdsDesc& vfds)
{
  ASKAPASSERT (vfds.antNr("RT0") == 0);
  ASKAPASSERT (vfds.antNr("RT1") == 1);
  ASKAPASSERT (vfds.antNr("RT2") == 2);
  ASKAPASSERT (vfds.antNr("RT3") == 3);
  ASKAPASSERT (vfds.antNr("RT4") == -1);
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT.*"));
    ASKAPASSERT (antNrs.size() == 4);
    ASKAPASSERT (antNrs[0] == 0);
    ASKAPASSERT (antNrs[1] == 1);
    ASKAPASSERT (antNrs[2] == 2);
    ASKAPASSERT (antNrs[3] == 3);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex(".*0"));
    ASKAPASSERT (antNrs.size() == 1);
    ASKAPASSERT (antNrs[0] == 0);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT2"));
    ASKAPASSERT (antNrs.size() == 1);
    ASKAPASSERT (antNrs[0] == 2);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT*"));
    ASKAPASSERT (antNrs.size() == 0);
  }
}

void doIt()
{
  VdsPartDesc vds;
  vds.setName ("/usr/local/xyx", "node1:/usr");
  vds.setTimes (0, 1);
  vds.addBand (64, 20, 100);
  vds.addBand (128, 120, 300);
  vector<int> ant1(3);
  ant1[0] = 0;
  ant1[1] = 1;
  ant1[2] = 2;
  vector<int> ant2(ant1);
  ant2[2] = 3;
  vds.setBaselines (ant1, ant2);
  vector<string> antNames(4);
  antNames[0] = "RT0";
  antNames[1] = "RT1";
  antNames[2] = "RT2";
  antNames[3] = "RT3";
  VdsDesc vfds(vds, antNames);
  vds.setTimes(0,2);
  vfds.addPart (vds);
  check(vfds);
  // Write into parset file.
  ofstream fos("tVdsDesc_tmp.fil");
  vfds.write (fos);
  // Read back.
  LOFAR::ACC::APS::ParameterSet parset("tVdsDesc_tmp.fil");
  VdsDesc vfds2(parset);
  check(vfds2);
  vfds = vfds2;
  check(vfds);
  tryAnt (vfds);
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
