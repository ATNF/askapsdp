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

#include  "GrothTriangles.h"
#include <assert.h>
#include <iostream>

using namespace askap::analysis::matching;

int main()
{

  Triangle *t1, *t2, *t3, *t4, *t5, *t6;

  t1 = new Triangle(4.,2.,5.,9.,1.,4.);
  t2 = new Triangle(8.,7.,14.,4.,12.,3.);
  t3 = new Triangle(8.,14.,14.,17.,12.,18.);
  t4 = new Triangle(1.,22.,4.,24.,5.,17.);

  // this one is t4 scaled by 2
  t5 = new Triangle(2.,44.,8.,48.,10.,34.);

  // get this one by rotating t4 by 90 and translating to new axes
  t6 = new Triangle(4., 1., 2., 4., 9., 5.);
  
  assert( (pow(10,t1->perimeter())-17.079)<0.001 );
  assert( (pow(10,t2->perimeter())-14.601)<0.001 );
  assert( (pow(10,t3->perimeter())-14.601)<0.001 );
  assert( (pow(10,t4->perimeter())-17.079)<0.001 );
  assert( (pow(10,t5->perimeter())-34.159)<0.001 );
  assert( (pow(10,t6->perimeter())-17.079)<0.001 );

  assert( t1->isClockwise() );
  assert( t2->isClockwise() );
  assert( !t3->isClockwise() );
  assert( !t4->isClockwise() );
  assert( !t5->isClockwise() );
  assert( !t6->isClockwise() );

  assert( t1->isMatch(*t4) );
  assert( t3->isMatch(*t2) );
  assert( !t1->isMatch(*t2) );
  assert( t1->isMatch(*t5) );
  assert( t1->isMatch(*t6) );

  std::cout << "ALL WORKED!\n";

}
