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

#include <simulationutilities/Continuum.h>

namespace askap {

  namespace simulations {

    const std::string ContinuumInput="187.5 -45. 0. 0.5 0. 10. 5. 30.";

    class ContinuumTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ContinuumTest);
	CPPUNIT_TEST(testParameters);
	CPPUNIT_TEST(testFluxes);
	CPPUNIT_TEST_SUITE_END();
	
    private:
	// members
	Continuum itsComponent;
	
    public:

	void setUp(){

	  itsComponent.define(ContinuumInput);

	}

/*****************************************/
	void tearDown() {
	}

/*****************************************/
	void testParameters() {
	  CPPUNIT_ASSERT(itsComponent.ra()=="187.5");
	  CPPUNIT_ASSERT(itsComponent.dec()=="-45.");
	  CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-1.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.alpha()-0.5)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.maj()-10.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.min()-5.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.pa()-30.)<1.e-6);
	}

	void testFluxes(){
	  CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-1.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(1400.)-1.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(1000.)-0.84515425)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(2000.)-1.1952286)<1.e-6);
	}

    };
  }
}
