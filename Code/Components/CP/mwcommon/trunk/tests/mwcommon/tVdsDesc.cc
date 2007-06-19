//# tVdsDesc.cc: Test program for class VdsDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <mwcommon/VdsDesc.h>
#include <mwcommon/ConradError.h>
#include <ostream>
#include <fstream>

using namespace conrad::cp;
using namespace casa;
using namespace std;

void checkVds (const VdsPartDesc& vds, double endTime)
{
  CONRADASSERT (vds.getName() == "/usr/local/xyx");
  CONRADASSERT (vds.getFileSys() == "node1:/usr");
  CONRADASSERT (vds.getStartTime() == 0);
  CONRADASSERT (vds.getEndTime() == endTime);
  CONRADASSERT (vds.getNChan().size() == 2);
  CONRADASSERT (vds.getNChan()[0] == 64);
  CONRADASSERT (vds.getNChan()[1] == 128);
  CONRADASSERT (vds.getStartFreqs().size() == 2);
  CONRADASSERT (vds.getStartFreqs()[0] == 20);
  CONRADASSERT (vds.getStartFreqs()[1] == 120);
  CONRADASSERT (vds.getEndFreqs().size() == 2);
  CONRADASSERT (vds.getEndFreqs()[0] == 100);
  CONRADASSERT (vds.getEndFreqs()[1] == 300);
  CONRADASSERT (vds.getAnt1().size() == 3);
  CONRADASSERT (vds.getAnt1()[0] == 0);
  CONRADASSERT (vds.getAnt1()[1] == 1);
  CONRADASSERT (vds.getAnt1()[2] == 2);
  CONRADASSERT (vds.getAnt2().size() == 3);
  CONRADASSERT (vds.getAnt2()[0] == 0);
  CONRADASSERT (vds.getAnt2()[1] == 1);
  CONRADASSERT (vds.getAnt2()[2] == 3);
}

void check (const VdsDesc& vfds)
{
  checkVds (vfds.getDesc(), 1);
  checkVds (vfds.getPart(0), 2);
  CONRADASSERT (vfds.getAntNames().size() == 4);
  CONRADASSERT (vfds.getAntNames()[0] == "RT0");
  CONRADASSERT (vfds.getAntNames()[1] == "RT1");
  CONRADASSERT (vfds.getAntNames()[2] == "RT2");
  CONRADASSERT (vfds.getAntNames()[3] == "RT3");
}

void tryAnt (const VdsDesc& vfds)
{
  CONRADASSERT (vfds.antNr("RT0") == 0);
  CONRADASSERT (vfds.antNr("RT1") == 1);
  CONRADASSERT (vfds.antNr("RT2") == 2);
  CONRADASSERT (vfds.antNr("RT3") == 3);
  CONRADASSERT (vfds.antNr("RT4") == -1);
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT.*"));
    CONRADASSERT (antNrs.size() == 4);
    CONRADASSERT (antNrs[0] == 0);
    CONRADASSERT (antNrs[1] == 1);
    CONRADASSERT (antNrs[2] == 2);
    CONRADASSERT (antNrs[3] == 3);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex(".*0"));
    CONRADASSERT (antNrs.size() == 1);
    CONRADASSERT (antNrs[0] == 0);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT2"));
    CONRADASSERT (antNrs.size() == 1);
    CONRADASSERT (antNrs[0] == 2);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT*"));
    CONRADASSERT (antNrs.size() == 0);
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
