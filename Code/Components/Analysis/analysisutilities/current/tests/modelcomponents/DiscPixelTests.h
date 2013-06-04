/// @file
///
/// Unit tests for DiscEllipse class
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

#include <iostream>
#include <sstream>

namespace askap {

    namespace analysisutilities {
    
 
	class DiscPixelTest : public CppUnit::TestFixture {
	    CPPUNIT_TEST_SUITE(DiscPixelTest);
	    CPPUNIT_TEST(testFluxesOut);
	    CPPUNIT_TEST(testFluxesIn);
	    CPPUNIT_TEST(testFluxesEdge);
	    CPPUNIT_TEST_SUITE_END();
	
	private:
	    // members
	    DiscEllipse itsEllipse;
	    std::vector<DiscPixel> itsPixels;
	    double x0,y0,maj,min,pa;

	    int pixOut,pixIn,pixEdge;
	    int xOut,yOut,xIn,yIn,xEdge,yEdge;
	    double fluxOut,fluxIn,fluxEdge;

	public:

	    void setUp(){
		x0=5.;
		y0=6.;
		maj=12./M_PI;
		min=2.5;
		pa=M_PI/6.;

		itsEllipse=DiscEllipse(x0,y0,maj,min,pa);

		itsPixels=itsEllipse.boundingSet(1000);

		pixOut=2; xOut=3; yOut=2; fluxOut=0.;
		pixIn=31; xIn=5; yIn=5; fluxIn=1.;
		pixEdge=13; xEdge=5; yEdge=3; fluxEdge=0.79102;


	    }

	    void tearDown(){
	    }

	    void testFluxesOut(){
		CPPUNIT_ASSERT(itsPixels[pixOut].x()==xOut);
		CPPUNIT_ASSERT(itsPixels[pixOut].y()==yOut);
		CPPUNIT_ASSERT(!itsEllipse.isIn(xOut,yOut));
		CPPUNIT_ASSERT(fabs(itsPixels[pixOut].flux()-fluxOut)<1.e-5);
	    }

	    void testFluxesIn(){
		CPPUNIT_ASSERT(itsPixels[pixIn].x()==xIn);
		CPPUNIT_ASSERT(itsPixels[pixIn].y()==yIn);
		CPPUNIT_ASSERT(itsEllipse.isIn(xIn,yIn));
		CPPUNIT_ASSERT(fabs(itsPixels[pixIn].flux()-fluxIn)<1.e-5);
	    }

	    void testFluxesEdge(){
		CPPUNIT_ASSERT(itsPixels[pixEdge].x()==xEdge);
		CPPUNIT_ASSERT(itsPixels[pixEdge].y()==yEdge);
		CPPUNIT_ASSERT(fabs(itsPixels[pixEdge].flux()-fluxEdge)<1.e-5);
	    }

	};

    }

}
