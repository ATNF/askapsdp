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

#include <simulationutilities/ContinuumS3SEX.h>

#include <iostream>
#include <sstream>

namespace askap {

  namespace simulations {
    

    class ContinuumS3SEXTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ContinuumS3SEXTest);
	CPPUNIT_TEST(testParameters);
	CPPUNIT_TEST(testFluxes);
	CPPUNIT_TEST_SUITE_END();
	
    private:
	// members
	ContinuumS3SEX itsComponent;
	
    public:

	float getFlux(float nu){return i_1400+log10(pow((nu/nu0),alpha+beta*log(nu/nu0)));};
	std::string component;
	int structure;
	std::string right_ascension;
	std::string declination;
	float position_angle;
	float major_axis;
	float minor_axis;
	float alpha;
	float beta;
	float nu0;
	float i_1400;
	float i_151;
	float i_610;
	float i_4860;
	float i_18000;

	void setUp(){

	// component    galaxy structure right_ascension declination position_angle major_axis minor_axis   i_151   i_610  i_1400  i_4860 i_18000
	//    const std::string ContinuumS3SEXInput="12205907  12205907         1      180.343780  -49.569685            0.0        0.0        0.0 -5.3230 -5.7474 0.0000 -6.3784 -6.7764";
	  component="12205907";
	  structure=1;
	  right_ascension="187.5";
	  declination="-45.";
	  position_angle=30.;
	  major_axis=10.;
	  minor_axis=5.;
	  alpha=0.5;
	  beta=0.1;
	  nu0=1400.;
	  i_1400=0.;
	  i_151=getFlux(151.);
	  i_610=getFlux(610.);
	  i_4860=getFlux(4860.);
	  i_18000=getFlux(18000.);
	
	  std::ostringstream ContinuumS3SEXInput;
	  ContinuumS3SEXInput << component << " " << 
	    component << " " << 
	    structure << " " << 
	    right_ascension << " " << 
	    declination << " " << 
	    position_angle << " " << 
	    major_axis << " " << 
	    minor_axis << " " << 
	    i_151 << " " << 
	    i_610 << " " << 
	    i_1400 << " " << 
	    i_4860 << " " <<
	    i_18000;
	  itsComponent.setNuZero(1.4e9);
	  itsComponent.define(ContinuumS3SEXInput.str());
	  itsComponent.prepareForUse();

	}

/*****************************************/
	void tearDown() {
	}

/*****************************************/
	void testParameters() {
	  CPPUNIT_ASSERT(itsComponent.ra()=="187.5");
	  CPPUNIT_ASSERT(itsComponent.dec()=="-45.");
	  CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-pow(10,i_1400))<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.alpha()-alpha)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.maj()-major_axis)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.min()-minor_axis)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.pa()-position_angle)<1.e-6);
	}

	void testFluxes(){
	  CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-pow(10,i_1400))<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(1400.e6)-pow(10.,i_1400))<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(1000.e6)-0.8547769)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(2000.e6)-1.2105311)<1.e-6);
	}

    };
  }
}
