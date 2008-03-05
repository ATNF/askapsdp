//#  tSocketConnection.cc: Program to test SocketConnection
//#
//#  Copyright (C) 2007
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
