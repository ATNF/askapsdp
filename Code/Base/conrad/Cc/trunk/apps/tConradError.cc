//#  tConradError.cc: Test program for ConradError.h
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <conrad/ConradError.h>
#include <iostream>

using namespace conrad;
using namespace std;

void assert0 (int x)
{
  cout << "assert0..." << endl;
  CONRADASSERT (x==0);
}

void assert1 (int x)
{
  cout << "assert1..." << endl;
  try {
    CONRADASSERT (x==1);
    CONRADASSERT (0==1);       // should not be called
  } catch (AssertError& ex) {
    cout << ex.what() << endl;
  }
}

void assert2 (float y)
{
  cout << "assert2..." << endl;
  try {
    CONRADASSERT (y==2);
    CONRADASSERT (0==2);       // should not be called
  } catch (ConradError& ex) {
    cout << ex.what() << endl;
  }
}

void assert3 (double x)
{
  cout << "assert3..." << endl;
  try {
    CONRADASSERT (x==3);
    CONRADASSERT (0==3);       // should not be called
  } catch (exception& ex) {
    cout << ex.what() << endl;
  }
}


void check0 (int x)
{
  cout << "check0..." << endl;
  CONRADCHECK (x==0, "check0");
}

void check1 (int x)
{
  cout << "check1..." << endl;
  try {
    CONRADCHECK (x==1, "check1");
    CONRADCHECK (0==1, "check01");       // should not be called
  } catch (CheckError& ex) {
    cout << ex.what() << endl;
  }
}

void check2 (float y)
{
  cout << "check2..." << endl;
  try {
    CONRADCHECK (y==2, "check2");
    CONRADCHECK (0==2, "check02");       // should not be called
  } catch (ConradError& ex) {
    cout << ex.what() << endl;
  }
}

void check3 (double x)
{
  cout << "check3..." << endl;
  try {
    CONRADCHECK (x==3, "check3");
    CONRADCHECK (0==3, "check03");       // should not be called
  } catch (exception& ex) {
    cout << ex.what() << endl;
  }
}


int main()
{
  try {
    assert0 (0);
    assert1 (0);
    assert2 (1);
    assert3 (2);
    check0 (0);
    check1 (0);
    check2 (1);
    check3 (2);
  } catch (exception& x) {
    cout << "Unexpected end: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}

