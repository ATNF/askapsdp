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

#include <simulationutilities/ContinuumSelavy.h>

namespace askap {

  namespace simulations {

	  ///#   ID           Name         RA        DEC          F_int        F_peak          F_int(fit)           F_pk(fit)  Maj(fit)  Min(fit) P.A.(fit) Maj(fit_deconv.) Min(fit_deconv.) P.A.(fit_deconv.)      Alpha       Beta                 Chisq(fit)          RMS(image)       RMS(fit) Nfree(fit) NDoF(fit) NPix(fit) NPix(obj) Guess?
    const std::string ContinuumSelavyInput="    1a J124537-450659 187.5 -45.     0.12673451     0.04296888          1.          0.03728425   10. 5. 30. 10. 5. 30.      0.5      0.000               45.244525909          0.00452679          0.75203496          6        73        80        80      0";

    class ContinuumSelavyTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ContinuumSelavyTest);
	CPPUNIT_TEST(testParameters);
	CPPUNIT_TEST(testFluxes);
	CPPUNIT_TEST_SUITE_END();
	
    private:
	// members
	ContinuumSelavy itsComponent;
	
    public:

	void setUp(){

	  itsComponent.define(ContinuumSelavyInput);

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
	  CPPUNIT_ASSERT(!itsComponent.isGuess());
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
