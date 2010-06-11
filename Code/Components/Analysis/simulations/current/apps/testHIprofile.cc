///
/// @file : Create a FITS file with fake sources and random noise
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// @copyright (c) 2007 CSIRO
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
#include <askap_simulations.h>

#include <simulationutilities/HIprofile.h>
#include <simulationutilities/HIprofileS3SEX.h>
#include <simulationutilities/HIprofileS3SAX.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>

#include <FITS/FITSparallel.h>
#include <FITS/FITSfile.h>

#include <Common/ParameterSet.h>
#include <casa/OS/Timer.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <time.h>

using namespace askap;
using namespace askap::simulations;
using namespace askap::simulations::FITS;

ASKAP_LOGGER(logger, "testHIprofile.log");

// Move to Askap Util?
std::string getInputs(const std::string& key, const std::string& def, int argc,
                      const char** argv)
{
    if (argc > 2) {
        for (int arg = 0; arg < (argc - 1); arg++) {
            std::string argument = std::string(argv[arg]);

            if (argument == key) {
                return std::string(argv[arg+1]);
            }
        }
    }

    return def;
}

// Main function
int main(int argc, const char** argv)
{
    std::ifstream config("askap.log_cfg", std::ifstream::in);

    if (config) {
        ASKAPLOG_INIT("askap.log_cfg");
    } else {
        std::ostringstream ss;
        ss << argv[0] << ".log_cfg";
        ASKAPLOG_INIT(ss.str().c_str());
    }

    try {

        srandom(time(0));

        float mHI = 8.516200;
        float z = 0.005453;
        float maj = 47.064;
        float min =  6.275;
        //      float pa = 0.314;

        HIprofileS3SEX prof;
        prof.setup(SFG, z, mHI, maj, min);
        std::cout << prof << "\n";

        std::ofstream dumpfile("testHIprofileS3SEX_dump1.txt");
        int nchan = 200;
        double nuMax = 1414.e6;
        double deltaNu = 18.3e3;

        for (int i = 0; i < nchan; i++) {
            double nu = nuMax - i * deltaNu;
            double f = prof.flux(nu);
            dumpfile << i << "\t" << nu << "\t" << freqToHIVel(nu) - redshiftToVel(z) << "\t" << f << "\n";
        }

        dumpfile.close();

        dumpfile.open("testHIprofileS3SEX_dump2.txt");

        for (int i = 0; i < nchan; i++) {
            double nu = nuMax - i * deltaNu;
            double f = prof.flux(nu - deltaNu / 2., nu + deltaNu / 2.);
            dumpfile << i << "\t" << nu << "\t" << freqToHIVel(nu) - redshiftToVel(z) << "\t" << f << "\n";
        }

        dumpfile.close();

//-----------------

//        std::string inputLine = " -0.145550   0.004051   0.016469   0.000000   0.000000   0.012999   0.012950   0.641905   0.005453 566092000.000000   0.049868   0.049868   7.338150  26.933600  37.023300"; // A source from the S3SAX database, with the redshift changed to match the value above.
//        std::string inputLine = " -0.019340   0.656939   3.338000   0.000000   0.000000  58.540500  55.164758   0.990367   0.005453 1548270000.000000   0.013327   0.025629  45.206100  66.714800  77.308300";
	std::string inputLine = " -0.019340   0.656939   7.448240   0.000000   0.000000   0.000000   0.000000   0.343044   0.010938 158415000.000000   0.0133269  0.025629  45.206100  66.714800  77.308300";
        HIprofileS3SAX prof2(inputLine);
        std::cout << prof2 << "\n";
        dumpfile.open("testHIprofileS3SAX_dump1.txt");

        for (int i = 0; i < nchan; i++) {
            double nu = nuMax - i * deltaNu;
            double f = prof2.flux(nu);
            dumpfile << i << "\t" << nu << "\t" << freqToHIVel(nu) - redshiftToVel(z) << "\t"  << f << "\n";
        }

        dumpfile.close();

        dumpfile.open("testHIprofileS3SAX_dump2.txt");

        for (int i = 0; i < nchan; i++) {
            double nu = nuMax - i * deltaNu;
            double f = prof2.flux(nu - deltaNu / 2., nu + deltaNu / 2.);
            dumpfile << i << "\t" << nu << "\t" << freqToHIVel(nu) - redshiftToVel(z) << "\t" << f << "\n";
        }

        dumpfile.close();

//  ASKAPLOG_INFO_STR(logger, "Time for execution of testHIprofile = " << timer.real() << " sec");

    } catch (askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    exit(0);
}

