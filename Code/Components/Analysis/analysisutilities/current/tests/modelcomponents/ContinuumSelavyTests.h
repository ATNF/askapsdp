/// @file
///
/// Unit tests for the ContinuumSelavy class
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

#include <modelcomponents/ContinuumSelavy.h>

namespace askap {

  namespace analysisutilities {

//Original input was as follows - just the header of a selavy-fitResults.txt file, with a single line
/* 
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
#      ID           Name         RA        DEC      X      Y       F_int      F_peak   F_int(fit)   F_pk(fit)  Maj(fit)  Min(fit) P.A.(fit)  Maj(fit_deconv.)   Min(fit_deconv.) P.A.(fit_deconv.)                          Alpha    Beta Chisq(fit)   RMS(image)  RMS(fit) Nfree(fit) NDoF(fit) NPix(fit) NPix(obj) Guess? 
#                             [deg]      [deg]                      [Jy]   [Jy/beam]         [Jy]   [Jy/beam]  [arcsec]  [arcsec]     [deg]          [arcsec]           [arcsec]             [deg]                                                      [Jy/beam]                                                           
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
      70a J034439-362734  56.159180 -36.463321 1362.0 4235.7     0.08043     0.03443      0.04286     0.03440    25.425    17.936      4.97            9.9803            6.93991              7.84                       -1.06685   0.000    787.936     0.000258     1.195          6       533       552       552      0
*/ 

      const std::string ContinuumSelavyInput="      70a J034439-362734  56.159180 -36.463321 1362.0 4235.7     0.08043     0.03443      0.04286     0.03440    25.425    17.936      4.97            9.9803            6.93991              7.84                       -1.06685   0.000    787.936     0.000258     1.195          6       533       552       552      0";

      const std::string RA = ContinuumSelavyInput.substr(26,9);
      const std::string Dec = ContinuumSelavyInput.substr(36,10);
      const double flux = atof(ContinuumSelavyInput.substr(90,7).c_str());
      const double alpha = atof(ContinuumSelavyInput.substr(217,8).c_str());
      const double maj = atof(ContinuumSelavyInput.substr(113,6).c_str());
      const double min = atof(ContinuumSelavyInput.substr(123,6).c_str());
      const double pa = atof(ContinuumSelavyInput.substr(135,4).c_str());


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
	      CPPUNIT_ASSERT(itsComponent.ra()==RA);
	      CPPUNIT_ASSERT(itsComponent.dec()==Dec);
	      CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-flux)<1.e-6);
	      CPPUNIT_ASSERT(fabs(itsComponent.alpha()-alpha)<1.e-6);
	      CPPUNIT_ASSERT(fabs(itsComponent.maj()-maj)<1.e-6);
	      CPPUNIT_ASSERT(fabs(itsComponent.min()-min)<1.e-6);
	      CPPUNIT_ASSERT(fabs(itsComponent.pa()-pa)<1.e-6);
	      CPPUNIT_ASSERT(!itsComponent.isGuess());
	  }

	  void testFluxes(){
	      const double f1400 = flux * pow(1400./itsComponent.nuZero(), alpha);
	      const double f1000 = flux * pow(1000./itsComponent.nuZero(), alpha);
	      const double f2000 = flux * pow(2000./itsComponent.nuZero(), alpha);

	      CPPUNIT_ASSERT(fabs(itsComponent.fluxZero()-flux)<1.e-6);
	      CPPUNIT_ASSERT(fabs(itsComponent.flux(1400.)-f1400)<1.e-6);
	      CPPUNIT_ASSERT(fabs(itsComponent.flux(1000.)-f1000)<1.e-6);
	      CPPUNIT_ASSERT(fabs(itsComponent.flux(2000.)-f2000)<1.e-6);
	  }

      };
  }
}
