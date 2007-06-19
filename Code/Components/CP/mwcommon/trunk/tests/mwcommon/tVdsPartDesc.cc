//# tVdsPartDesc.cc: Test program for class VdsPartDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <mwcommon/VdsPartDesc.h>
#include <mwcommon/ConradError.h>
#include <ostream>
#include <fstream>

using namespace conrad::cp;
using namespace std;

void check (const VdsPartDesc& vds)
{
  CONRADASSERT (vds.getName() == "/usr/local/xyx");
  CONRADASSERT (vds.getFileSys() == "node1:/usr");
  CONRADASSERT (vds.getStartTime() == 0);
  CONRADASSERT (vds.getEndTime() == 1);
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
  check(vds);
  // Write into parset file.
  ofstream fos("tVdsPartDesc_tmp.fil");
  vds.write (fos, "");
  // Read back.
  LOFAR::ACC::APS::ParameterSet parset("tVdsPartDesc_tmp.fil");
  VdsPartDesc vds2(parset);
  check(vds2);
  vds = vds2;
  check(vds);
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
