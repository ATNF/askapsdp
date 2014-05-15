/// @file
///
/// Unit tests for Disc class
///
/// @copyright (c) 2008 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <askap_analysisutilities.h>
#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <modelcomponents/Ellipse.h>
#include <modelcomponents/DiscEllipse.h>
#include <modelcomponents/DiscPixel.h>
#include <modelcomponents/Disc.h>

#include <iostream>
#include <sstream>

namespace askap {

    namespace analysisutilities {
    
 
	class DiscTest : public CppUnit::TestFixture {
	    CPPUNIT_TEST_SUITE(DiscTest);
	    CPPUNIT_TEST(testRanges);
	    CPPUNIT_TEST(testFluxes);
	    CPPUNIT_TEST_SUITE_END();
	
	private:
	    // members
	    Disc itsDisc;
	    double x0,y0,maj,min,pa;
	    int xmin, xmax, ymin, ymax;
	    int xOut,yOut,xIn,yIn,xEdge,yEdge;
	    double fluxOut,fluxIn,fluxEdge,area;

	public:

	    void setUp(){
		x0=5.;
		y0=6.;
		maj=12./M_PI;
		min=2.5;
		pa=M_PI/6.;

		itsDisc.setup(x0,y0,maj,min,pa);

		xmin=1;
		xmax=9;
		ymin=2;
		ymax=10;

		xOut=3; yOut=2; fluxOut=0.;
		xIn=5; yIn=5; fluxIn=1.;
		xEdge=5; yEdge=3; fluxEdge=0.79102;
		
		area=M_PI*maj*min;

	    }

	    void tearDown(){
	    }

	    void testRanges(){
		CPPUNIT_ASSERT(itsDisc.xmin()==xmin);
		CPPUNIT_ASSERT(itsDisc.xmax()==xmax);
		CPPUNIT_ASSERT(itsDisc.ymin()==ymin);
		CPPUNIT_ASSERT(itsDisc.ymax()==ymax);
	    }

	    void testFluxes(){
		CPPUNIT_ASSERT(fabs(itsDisc.flux(xOut,yOut)-fluxOut/area)<1.e-5);
		CPPUNIT_ASSERT(fabs(itsDisc.flux(xIn,yIn)-fluxIn/area)<1.e-5);
		CPPUNIT_ASSERT(fabs(itsDisc.flux(xEdge,yEdge)-fluxEdge/area)<1.e-5);

	    }


	};

    }

}
