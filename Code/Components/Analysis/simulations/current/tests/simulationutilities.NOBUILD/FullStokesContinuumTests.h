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

#include <simulationutilities/FullStokesContinuum.h>

#include <iostream>
#include <sstream>

namespace askap {

  namespace simulations {
    
 
    class FullStokesContinuumTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(FullStokesContinuumTest);
	CPPUNIT_TEST(testParameters);
	CPPUNIT_TEST(testFluxes);
	CPPUNIT_TEST(testPol);
	CPPUNIT_TEST_SUITE_END();
	
    private:
	// members
	FullStokesContinuum itsComponent;
	
    public:

	int source, cluster, galaxy, sftype,agntype, structure,rmflag;
	std::string right_ascension, declination;
	float distance, redshift, position_angle, major_axis, minor_axis, alpha, beta, nu0, i_1420, i_151, i_610, i_4860, i_18000, refpolangle, polflux,polfrac, q_1420, u_1420, cosva, RM;
	float getFlux(float nu){return i_1420+log10(pow((nu/nu0),alpha+beta*log(nu/nu0)));};

	void setUp(){

	//36132603      0   36132579  0  2  1    -1.18603    -0.20153     33.342   0.007799     0.000     0.000     0.000   -5.0413   -4.8179  1.6361e-05 -4.0670e-06  3.4060e-06  5.3048e-06    0.3242   -4.8799   -5.1615    0.4973    -9.7981     0.0000
	  source=36132603;
	  cluster=0;
	  galaxy=36132579;
	  sftype=0;
	  agntype=2;
	  structure=1;
	  right_ascension="187.5";
	  declination="-45.";
	  distance=33.342;
	  redshift=0.007799;
	  position_angle=30.;
	  major_axis=10.;
	  minor_axis=5.;
	  alpha=0.5;
	  beta=0.1;
	  nu0=1420.e6;
	  i_1420=0.;
	  i_151=getFlux(freqValuesS3SEX[0]);
	  i_610=getFlux(freqValuesS3SEX[1]);
	  i_4860=getFlux(freqValuesS3SEX[3]);
	  i_18000=getFlux(freqValuesS3SEX[4]);
	  refpolangle=22.5;
	  polfrac=0.1;
	  polflux=polfrac*pow(10,i_1420);
	  q_1420 = polflux * cos( 2.*refpolangle*M_PI/180.);
	  u_1420 = polflux * sin( 2.*refpolangle*M_PI/180.);
	  cosva=0.4973;
	  RM=-9.7981;
	  rmflag=0;
	  std::ostringstream FullStokesContinuumInput;
	  FullStokesContinuumInput << source << " " << cluster << " " << galaxy << " " << sftype << " " << agntype << " " << structure << " " << 
	    right_ascension << " " <<  declination << " " << distance << " " << redshift << " " <<
	    position_angle << " " << major_axis << " " << minor_axis << " " << 
	    i_151 << " " << i_610 << " " << pow(10,i_1420) << " " << q_1420 << " " << u_1420 << " " << polflux << " " << polfrac << " " <<
	    i_4860 << " " << i_18000 << " " << cosva <<" " << RM << " " << rmflag;
	  itsComponent.setNuZero(1.42e9);
	  itsComponent.define(FullStokesContinuumInput.str());
	  itsComponent.prepareForUse();

	}

/*****************************************/
	void tearDown() {
	}

/*****************************************/
	void testParameters() {
	  CPPUNIT_ASSERT(itsComponent.ra()==this->right_ascension);
	  CPPUNIT_ASSERT(itsComponent.dec()==this->declination);
	  CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-pow(10,this->i_1420))<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.alpha()-this->alpha)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.maj()-this->major_axis)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.min()-this->minor_axis)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.pa()-this->position_angle)<1.e-6);
	}

	void testFluxes(){
	  CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-pow(10,this->i_1420))<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(POLREFFREQ,0)-pow(10.,this->i_1420))<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(1000.e6,0)-0.84956365221)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(2000.e6,0)-1.20078452696)<1.e-6);
	}

	void testPol(){
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(POLREFFREQ,1)-this->q_1420)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(POLREFFREQ,2)-this->u_1420)<1.e-6);
	  CPPUNIT_ASSERT(fabs(itsComponent.flux(POLREFFREQ,3)-0.)<1.e-6);
	}
    };
  }
}
