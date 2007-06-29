//#  tMWStep.cc: test program for the MWStep class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: tMWStep.cc,v 1.6 2006/10/03 15:54:07 loose Exp $

#include <mwcontrol/MWMultiSpec.h>
#include <mwcontrol/ParameterHandlerBBS.h>

using namespace LOFAR::ACC::APS;
using namespace conrad::cp;
using namespace std;

int main()
{
  try {
    ParameterHandlerBBS psh (ParameterSet("tMWSpec.in"));
    psh.getSteps("Strategy").print (cout, "");
    cout << endl;
  } catch (LOFAR::Exception& e) {
    cout << "Unexpected: " << e.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
