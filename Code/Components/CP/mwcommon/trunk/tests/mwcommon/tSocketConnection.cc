//#  tSocketConnection.cc: Program to test SocketConnection
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

#include <mwcommon/SocketConnection.h>
#include <mwcommon/SocketListener.h>
#include <mwcommon/AskapError.h>
#include <iostream>
#include <unistd.h>

using namespace askap;
using namespace askap::cp;
using namespace std;

void doClient (const string& host, const string& port)
{
  //  sleep (2);
  cout << "Client connection on host " << host
       << ", port " << port << endl;
  SocketConnection socket(host, port);
  double dv = 1;
  socket.send (&dv, sizeof(dv));
  cout << "sent " << dv << endl;
  float fv = 0;
  socket.receive (&fv, sizeof(fv));
  ASKAPASSERT (fv == 2);
  cout << "received " << fv << endl;
  sleep(2);
  socket.receive (&fv, sizeof(fv));
  ASKAPASSERT (fv == 3);
  cout << "received " << fv << endl;
  dv = 2;
  socket.send (&dv, sizeof(dv));
  cout << "sent " << dv << endl;
}

void doServer (const string& port)
{
  cout << "Server connection on port " << port << endl;
  SocketListener listener(port);
  SocketConnection::ShPtr socket = listener.accept();
  double dv = 0;
  socket->receive (&dv, sizeof(dv));
  ASKAPASSERT (dv == 1);
  cout << "received " << dv << endl;
  float fv = 2;
  socket->send (&fv, sizeof(fv));
  cout << "sent " << fv << endl;
  fv = 3;
  socket->send (&fv, sizeof(fv));
  cout << "sent " << fv << endl;
  socket->receive (&dv, sizeof(dv));
  ASKAPASSERT (dv == 2);
  cout << "received " << dv << endl;
}

int main(int argc, const char* argv[])
{
  int status = 0;
  try {
    if (argc < 2) {
      cerr << "Run as:" << endl;
      cerr << "  as server:    tSocketConnection port" << endl;
      cerr << "  as client:    tSocketConnection port host" << endl;
    } else if (argc == 2) {
      doServer (argv[1]);
    } else {
      doClient (argv[2], argv[1]);
    }
  } catch (std::exception& x) {
    cout << "Unexpected exception in " << argv[0] << ": " << x.what() << endl;
    status = 1;
  }
  exit(status);
}
