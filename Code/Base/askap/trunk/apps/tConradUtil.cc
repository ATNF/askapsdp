//#  tConradUtil.cc: Test program for ConradUtil.h
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <askap/ConradUtil.h>
#include <askap/ConradError.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>

using namespace conrad;
using namespace std;

void testCase()
{
  cout << "Test toUpper and toLower..." << endl;
  string s("The zip code of North Ryde in NSW is 2113");
  CONRADASSERT (toUpper(s) == "THE ZIP CODE OF NORTH RYDE IN NSW IS 2113");
  CONRADASSERT (toLower(s) == "the zip code of north ryde in nsw is 2113");
  CONRADASSERT (s == "The zip code of North Ryde in NSW is 2113");
}

void testStream()
{
  cout << "Test printing a container..." << endl;
  {
    vector<int> vi;
    for (int i=0; i<5; ++i) vi.push_back(i-1);
    ostringstream os;
    os << vi;
    CONRADASSERT (os.str() == "[-1,0,1,2,3]");
  }
  {
    list<double> vi;
    for (int i=0; i<5; ++i) vi.push_back(i-0.5);
    ostringstream os;
    os << vi;
    CONRADASSERT (os.str() == "[-0.5,0.5,1.5,2.5,3.5]");
  }
  {
    list<string> vs;
    vs.push_back("aap");   vs.push_back("noot");    vs.push_back("mies");
    vs.push_back("wim");   vs.push_back("zus");     vs.push_back("jet");
    ostringstream os;
    printContainer (os, vs, "  ", "(", ")");
    CONRADASSERT (os.str() == "(aap  noot  mies  wim  zus  jet)");
  }
}

int main()
{
  try {
    testCase();
    testStream();
  } catch (exception& x) {
    cout << "Unexpected end: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
