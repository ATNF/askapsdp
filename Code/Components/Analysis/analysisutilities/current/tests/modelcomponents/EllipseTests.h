/// @file
///
/// Unit tests for Ellipse class
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

#include <iostream>
#include <sstream>

ASKAP_LOGGER(logger, ".ellipsetests");

namespace askap {

    namespace analysisutilities {
    
 
	const double X0=5.,Y0=6.,MAJ=12./M_PI,MIN=2.5,PA=M_PI/6.;

	class EllipseTest : public CppUnit::TestFixture {
	    CPPUNIT_TEST_SUITE(EllipseTest);
	    CPPUNIT_TEST(testArea);
	    CPPUNIT_TEST(testIsIn);
	    CPPUNIT_TEST(testCoords);
	    CPPUNIT_TEST_SUITE_END();
	
	private:
	    // members
	    Ellipse itsEllipse;
	    double x0,y0,maj,min,pa;
	    double area, parametricx_t0,parametricy_t0, parametricx_t90,parametricy_t90;

	public:

	    void setUp(){
		x0=5.;
		y0=6.;
		maj=12./M_PI;
		min=2.5;
		pa=M_PI/6.;

		itsEllipse=Ellipse(x0,y0,maj,min,pa);

		area=M_PI*maj*min;
		parametricx_t0= x0 + maj*cos(pa+M_PI/2.);
		parametricy_t0= y0 + maj*sin(pa+M_PI/2.);
		parametricx_t90= x0 - min*sin(pa+M_PI/2.);
		parametricy_t90= y0 + min*cos(pa+M_PI/2.);
		
	    }
	
	    void tearDown(){
	    }

	    void testArea(){
		CPPUNIT_ASSERT(fabs(itsEllipse.area()-area)<1.e-6);
	    }

	    void testIsIn(){
		CPPUNIT_ASSERT(itsEllipse.isIn(3.5,6.5));
		CPPUNIT_ASSERT(!itsEllipse.isIn(1.5,6.5));
	    }

	    void testCoords(){
		CPPUNIT_ASSERT(fabs(itsEllipse.parametricX(0.)-parametricx_t0)<1.e-6);
		CPPUNIT_ASSERT(fabs(itsEllipse.parametricY(0.)-parametricy_t0)<1.e-6);
		CPPUNIT_ASSERT(fabs(itsEllipse.nonRotX(parametricx_t0,parametricy_t0)-maj)<1.e-6);
		CPPUNIT_ASSERT(fabs(itsEllipse.nonRotY(parametricx_t0,parametricy_t0))<1.e-6);
		CPPUNIT_ASSERT(fabs(itsEllipse.parametricX(M_PI/2.)-parametricx_t90)<1.e-6);
		CPPUNIT_ASSERT(fabs(itsEllipse.parametricY(M_PI/2.)-parametricy_t90)<1.e-6);
		CPPUNIT_ASSERT(fabs(itsEllipse.nonRotX(parametricx_t90,parametricy_t90))<1.e-6);
		CPPUNIT_ASSERT(fabs(itsEllipse.nonRotY(parametricx_t90,parametricy_t90)-min)<1.e-6);
	    }

	    };

	}

    }
