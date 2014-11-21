/// @file
///
/// Test program for RM synthesis
///
/// @copyright (c) 2014 CSIRO
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
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askapparallel/AskapParallel.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <complex.h>
#include <casa/BasicSL/Complex.h>

#include <Common/ParameterSet.h>

using namespace askap;
using namespace askap::analysis;

ASKAP_LOGGER(logger, "tRMsynth.log");

int main(int argc, const char *argv[])
{

    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, argv);
    try {
	// initialise with defaults
	LOFAR::ParameterSet parset;
	const int numPhiChan=2500;
	const float deltaPhi=25.;
	parset.replace(LOFAR::KVpair("numPhiChan",numPhiChan));
	parset.replace(LOFAR::KVpair("deltaPhi",deltaPhi));
	parset.replace("weightType","uniform");
	RMSynthesis rmsynth(parset);

	const int nchan=300;
	const float C_ms=299792458.0;
	casa::IPosition shape(1,nchan);
	casa::Vector<float> freq(nchan);
	casa::indgen<float>(freq, 700.e6, 1.e6);
	casa::Vector<float> wl = C_ms / freq;
	casa::Vector<float> lamsq = wl * wl;

	//std::cout << freq << "\n" << lamsq << "\n";

	const float RM=120.;
	const float phiZero = 0.;
	casa::Vector<float> phi = lamsq*RM + phiZero;
	casa::Vector<float> u = sin(2.F*phi);
	casa::Vector<float> q = cos(2.F*phi);
	casa::Vector<float> noise(shape,1.);
    
	rmsynth.calculate(lamsq,q,u,noise);

	casa::Vector<casa::Complex> fdf = rmsynth.fdf();
	casa::Vector<float> fdf_p = casa::amplitude(fdf);
	casa::Vector<float> phi_rmsynth = rmsynth.phi();
	// std::cout << fdf_p << "\n";
	ASKAPLOG_INFO_STR(logger, "Size of FDF = "<<fdf_p.size());
	float minFDF,maxFDF;
	casa::IPosition locMin,locMax;
	casa::minMax<float>(minFDF,maxFDF,locMin,locMax,fdf_p);
	ASKAPLOG_INFO_STR(logger, "Max of FDF is " << maxFDF);
	ASKAPLOG_INFO_STR(logger, "Max of FDF is at pixel " << locMax);
	ASKAPLOG_INFO_STR(logger, "Max of FDF is at phi=" << phi_rmsynth(locMax)<< " rad/m2");
	ASKAPLOG_INFO_STR(logger, "Middle of phi & FDF follows:");
	for(int i=numPhiChan/2-10;i<numPhiChan/2+10;i++) std::cout << phi_rmsynth[i] << "\t" << fdf_p[i] << "\n";
	casa::Vector<casa::Complex> rmsf = rmsynth.rmsf();
	casa::Vector<float> rmsf_p = casa::amplitude(rmsf);
	ASKAPLOG_INFO_STR(logger, "Size of RMSF = " << rmsf_p.size());
	casa::Vector<float> phi_rmsynth_rmsf = rmsynth.phi_rmsf();
	ASKAPLOG_INFO_STR(logger, "Middle of phi & RMSF follows:");
	for(int i=numPhiChan-10;i<numPhiChan+10;i++) std::cout << phi_rmsynth_rmsf[i] << "\t" << rmsf_p[i] << "\n";

	ASKAPLOG_INFO_STR(logger, "RMSF width = " << rmsynth.rmsf_width());
	ASKAPLOG_INFO_STR(logger, "Expected : " << 2. * sqrt(3) / (lamsq[0]-lamsq[nchan-1])
			  << " based on lamsq[0]="<<lamsq[0] << " and lamsq["<<nchan-1<<"]="<<lamsq[nchan-1]);

	std::ofstream fout("rmsynth.out");
	for(int i=0;i<numPhiChan;i++){
	    fout << phi_rmsynth[i] << " " << fdf_p[i] << " " << rmsf_p[i+numPhiChan/2] <<"\n";
	}
	fout.close();


    } catch (askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    // } catch (const duchamp::DuchampError& x) {
    //     ASKAPLOG_FATAL_STR(logger, "Duchamp error in " << argv[0] << ": " << x.what());
    //     std::cerr << "Duchamp error in " << argv[0] << ": " << x.what() << std::endl;
    //     exit(1);
    } catch (std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    exit(0);

}
