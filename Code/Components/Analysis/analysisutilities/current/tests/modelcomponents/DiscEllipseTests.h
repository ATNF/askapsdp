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
    
 
	class DiscEllipseTest : public CppUnit::TestFixture {
	    CPPUNIT_TEST_SUITE(DiscEllipseTest);
	    CPPUNIT_TEST(testBoundingSet);
	    CPPUNIT_TEST_SUITE_END();
	
	private:
	    // members
	    DiscEllipse itsEllipse;
	    double x0,y0,maj,min,pa;
	    int xmin, xmax, ymin, ymax;

	public:

	    void setUp(){
		x0=5.;
		y0=6.;
		maj=12./M_PI;
		min=2.5;
		pa=M_PI/6.;

		itsEllipse=DiscEllipse(x0,y0,maj,min,pa);

		xmin=1;
		xmax=9;
		ymin=2;
		ymax=10;

	    }

	    void tearDown(){
	    }

	    void testBoundingSet(){
		std::vector<DiscPixel> boundingset=itsEllipse.boundingSet(1000);
		int count=0;
		for(int y=ymin;y<=ymax;y++){
		    for(int x=xmin;x<=xmax;x++){
			CPPUNIT_ASSERT(boundingset[count].x()==x);
			CPPUNIT_ASSERT(boundingset[count].y()==y);
			count++;
		    }
		}
	    }

	};

    }

}
