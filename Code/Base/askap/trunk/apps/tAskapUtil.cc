//#  tAskapUtil.cc: Test program for ConradUtil.h
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
//#  $Id$

#include <askap/AskapUtil.h>
#include <askap/AskapError.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>

using namespace askap;
using namespace std;

void testCase()
{
  cout << "Test toUpper and toLower..." << endl;
  string s("The zip code of North Ryde in NSW is 2113");
  ASKAPASSERT (toUpper(s) == "THE ZIP CODE OF NORTH RYDE IN NSW IS 2113");
  ASKAPASSERT (toLower(s) == "the zip code of north ryde in nsw is 2113");
  ASKAPASSERT (s == "The zip code of North Ryde in NSW is 2113");
}

void testStream()
{
  cout << "Test printing a container..." << endl;
  {
    vector<int> vi;
    for (int i=0; i<5; ++i) vi.push_back(i-1);
    ostringstream os;
    os << vi;
    ASKAPASSERT (os.str() == "[-1,0,1,2,3]");
  }
  {
    list<double> vi;
    for (int i=0; i<5; ++i) vi.push_back(i-0.5);
    ostringstream os;
    os << vi;
    ASKAPASSERT (os.str() == "[-0.5,0.5,1.5,2.5,3.5]");
  }
  {
    list<string> vs;
    vs.push_back("aap");   vs.push_back("noot");    vs.push_back("mies");
    vs.push_back("wim");   vs.push_back("zus");     vs.push_back("jet");
    ostringstream os;
    printContainer (os, vs, "  ", "(", ")");
    ASKAPASSERT (os.str() == "(aap  noot  mies  wim  zus  jet)");
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
