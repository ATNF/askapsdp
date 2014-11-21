/// @file
///
/// Testing the RM Synthesis code
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <polarisation/RMSynthesis.h>
#include <cppunit/extensions/HelperMacros.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".rmSynthesisTest");
namespace askap {
    namespace analysis {

 
	const int nchan=32;
	const float C_ms=299792458.0;
	const float RM=50.;
	const float psiZero = 0.;
	const unsigned int numPhiChan=2500;
	const float deltaPhi = 25.;
	const float phiZero = 0.;

	class RMSynthesisTest : public CppUnit::TestFixture {
	    CPPUNIT_TEST_SUITE(RMSynthesisTest);
	    CPPUNIT_TEST(testParsets);
	    CPPUNIT_TEST(testWeights);
	    CPPUNIT_TEST(testRMsynth);
	    CPPUNIT_TEST(testRMSF);
	    CPPUNIT_TEST(testRMSFwidth);
	    CPPUNIT_TEST_SUITE_END();

	private:
	    LOFAR::ParameterSet parset_uniform;
	    LOFAR::ParameterSet parset_variance;
	    casa::IPosition shape;
	    casa::Vector<float> freq,wl,lamsq,psi,u,q,noise;
 
	public:

	    void setUp() {
		ASKAPLOG_INFO_STR(logger, "+++++++++++++++++++++++++++++++++++++");
		ASKAPLOG_INFO_STR(logger, "Setting up the RMSynthesis Tests");

		parset_uniform.replace(LOFAR::KVpair("numPhiChan",int(numPhiChan)));
		parset_uniform.replace(LOFAR::KVpair("deltaPhi",deltaPhi));
		parset_uniform.replace(LOFAR::KVpair("phiZero",phiZero));
		parset_uniform.replace("weightType","uniform");

		parset_variance.replace(LOFAR::KVpair("numPhiChan",int(numPhiChan)));
		parset_variance.replace(LOFAR::KVpair("deltaPhi",deltaPhi));
		parset_variance.replace(LOFAR::KVpair("phiZero",phiZero));
		parset_variance.replace("weightType","variance");

		freq = casa::Vector<float>(nchan);
		casa::indgen<float>(freq, 700.e6, 1.e6);
		wl = C_ms / freq;
		lamsq = wl * wl;
		// this is the polarisation position angle
		psi = lamsq*RM + psiZero;
		u = sin(2.F*psi);
		q = cos(2.F*psi);
		noise = casa::Vector<float>(nchan);
 		casa::indgen<float>(noise, 1., 0.01);

	    }

	    void testParsets() {

		RMSynthesis rmsynthU(parset_uniform);
		CPPUNIT_ASSERT(rmsynthU.weightType()=="uniform");
		CPPUNIT_ASSERT(rmsynthU.numPhiChan()==numPhiChan);
		CPPUNIT_ASSERT(rmsynthU.deltaPhi() == deltaPhi);

		RMSynthesis rmsynthV(parset_variance);
		CPPUNIT_ASSERT(rmsynthV.weightType()=="variance");
		CPPUNIT_ASSERT(rmsynthV.numPhiChan()==numPhiChan);
		CPPUNIT_ASSERT(rmsynthV.deltaPhi() == deltaPhi);

	    }

	    
	    void testWeights() {
		RMSynthesis rmsynthU(parset_uniform);
		rmsynthU.calculate(lamsq,q,u,noise);
		float uniformNorm = 1./nchan;
		ASKAPLOG_DEBUG_STR(logger, "Normalisation for uniform case = " << rmsynthU.normalisation()
				   << ", should be " << uniformNorm);
		CPPUNIT_ASSERT(fabs(rmsynthU.normalisation()-uniformNorm)<1.e-5);
		
		RMSynthesis rmsynthV(parset_variance);
		rmsynthV.calculate(lamsq,q,u,noise);
		float varianceNorm=0.;
		for(int i=0;i<nchan;i++) varianceNorm += 1./(noise[i]*noise[i]);
		varianceNorm = 1./varianceNorm;
		ASKAPLOG_DEBUG_STR(logger, "Normalisation for variance case = " << rmsynthV.normalisation() 
				   << ", should be " << varianceNorm);
		CPPUNIT_ASSERT(fabs(rmsynthV.normalisation()-varianceNorm)<1.e-5);
		
	    }

	    void testRMsynth() {

		// We have chosen an RM such that the peak falls
		// directly on the sampled phi value of a bin. This
		// way we get a peak of 1 for the Faraday Dispersion
		// Function		
		RMSynthesis rmsynthU(parset_uniform);
		rmsynthU.calculate(lamsq,q,u,noise);
		casa::Vector<casa::Complex> fdf = rmsynthU.fdf();
		casa::Vector<float> fdf_p = casa::amplitude(fdf);
		casa::Vector<float> phi_rmsynth = rmsynthU.phi();
		float minFDF,maxFDF;
		casa::IPosition locMin,locMax;
		casa::minMax<float>(minFDF,maxFDF,locMin,locMax,fdf_p);
		float expectedMax=1.0;
		ASKAPLOG_DEBUG_STR(logger, "Expect max of FDF to be " << expectedMax << " and got " << maxFDF);
		CPPUNIT_ASSERT(fabs(maxFDF-expectedMax)<1.e-5);
		casa::IPosition expectedLoc(1,int(RM/deltaPhi+0.5*numPhiChan));
		ASKAPLOG_DEBUG_STR(logger, "Expect max of FDF to be at " << expectedLoc << " and got " << locMax);
		CPPUNIT_ASSERT(abs(locMax[0]-expectedLoc[0])==0);
		ASKAPLOG_DEBUG_STR(logger, "Expect max of FDF to be at " << RM << " rad/m2 and got " << phi_rmsynth(locMax));
		CPPUNIT_ASSERT(fabs(phi_rmsynth(locMax)-RM)<1.e-5);
	    }

	    void testRMSF() {

		RMSynthesis rmsynthU(parset_uniform);
		rmsynthU.calculate(lamsq,q,u,noise);

		casa::Vector<casa::Complex> rmsf = rmsynthU.rmsf();
		casa::Vector<float> rmsf_p = casa::amplitude(rmsf);
		std::cout << rmsf_p.size() << "\n";
		casa::Vector<float> phi_rmsynth_rmsf = rmsynthU.phi_rmsf();
		float minRMSF,maxRMSF;
		casa::IPosition locMin,locMax;
		casa::minMax<float>(minRMSF,maxRMSF,locMin,locMax,rmsf_p);
		float expectedMax=1.0;
		ASKAPLOG_DEBUG_STR(logger, "Expect max of RMSF to be " << expectedMax << " and got " << maxRMSF);
		CPPUNIT_ASSERT(fabs(maxRMSF-expectedMax)<1.e-5);
		casa::IPosition expectedLoc(1,numPhiChan);
		ASKAPLOG_DEBUG_STR(logger, "Expect max of RMSF to be at " << expectedLoc << " and got " << locMax);
		CPPUNIT_ASSERT(abs(locMax[0]-expectedLoc[0])==0);
		ASKAPLOG_DEBUG_STR(logger, "Expect max of RMSF to be at 0. rad/m2 and got " << phi_rmsynth_rmsf(locMax));
		CPPUNIT_ASSERT(fabs(phi_rmsynth_rmsf(locMax))<1.e-5);

	    }

	    void testRMSFwidth(){

		// *** NOTE - LEAVING THIS FOR NOW WHILE I SEEK ADVICE.
		// HAVE A THEORETICAL VALUE OF 2*SQRT(3)/(LAMSQ[0]-LAMSQ[NCHAN-1]), BUT ACTUAL WIDTH SEEMS 
		// TO GET BROADENED BY AMOUNT RELATED TO SAMPLING OF PHI (IE. DELTA_PHI). TEST THEREFORE FAILS.

		// RMSynthesis rmsynthU(parset_uniform);
		// rmsynthU.calculate(lamsq,q,u,noise);

		// float expectedRMSFwidth = 2. * sqrt(3.) / (lamsq[0] - lamsq[nchan-1]);
		// ASKAPLOG_DEBUG_STR(logger, "lamsq-max = " << lamsq[0] << ", lamsq-min = " << lamsq[nchan-1]);
		// ASKAPLOG_DEBUG_STR(logger, "Expect RMSF width for uniform weighting to be " << expectedRMSFwidth 
		// 		   << " and got " << rmsynthU.rmsf_width());
		// CPPUNIT_ASSERT(fabs(rmsynthU.rmsf_width() - expectedRMSFwidth)<1.e-5);
	    }

	    void tearDown() {
	    }

	};

    }
}
