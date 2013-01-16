/// @file
///
/// XXX Notes on program XXX
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
#include <askap_analysis.h>
#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <simulationutilities/ContinuumNVSS.h>

namespace askap {

  namespace simulations {

    const std::string ContinuumNVSSInput=" 9.992630   -9.85688   -1.64153   210145 C0300M36  967.43  265.48 025029-370029  02 50 29.51 -37 00 29.6  0.12  1.5   1000.0     0.6    10.0     5.0  30.0                          0.24   6.2  0.44 35.7 Image";

    class ContinuumNVSSTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ContinuumNVSSTest);
	CPPUNIT_TEST(testParameters);
	CPPUNIT_TEST(testFluxes);
	CPPUNIT_TEST_SUITE_END();
	
    private:
	// members
	ContinuumNVSS itsComponent;
	
    public:

	void setUp(){

	  itsComponent.define(ContinuumNVSSInput);
	  
	}

/*****************************************/
	void tearDown() {
	}

/*****************************************/
	void testParameters() {
	  CPPUNIT_ASSERT(itsComponent.ra()=="02:50:29.51");
	  CPPUNIT_ASSERT(itsComponent.dec()=="-37:00:29.6");
	  CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-1.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.alpha()-0.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.maj()-10.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.min()-5.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.pa()-30.)<1.e-6);
	}

	void testFluxes(){
	  CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-1.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(1400.)-1.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(1000.)-1.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(2000.)-1.)<1.e-6);
	}

    };
  }
}
