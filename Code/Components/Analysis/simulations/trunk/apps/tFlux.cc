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

#include <FITS/FITSfile.h>
#include <simulationutilities/FluxGenerator.h>
#include <simulationutilities/Continuum.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>

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

ASKAP_LOGGER(logger, "tFlux.log");

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
    ASKAPLOG_INIT("tFlux.log_cfg");

    LOFAR::ParameterSet parset("tests/tparset.in");
    parset = parset.makeSubset("createFITS.");
    FITSfile file(parset);

    struct wcsprm *wcs = file.getWCS();
//     wcsprt(wcs);

    std::vector<int> axes = parset.getInt32Vector("axes");
    int nz = axes[wcs->spec];
     FluxGenerator fluxes(nz);
    ASKAPLOG_DEBUG_STR(logger, "number of channels = " << nz);
    Continuum cont(-1.,-1.,1.4e9,1.);
    double x=512.,y=512.;
    fluxes.addSpectrum(cont,x,y,wcs);

    for(int i=0;i<fluxes.nChan();i++)
      std::cout << i << " " << fluxes.getFlux(i) << "\n";

    std::cout << "\n";
    
    FluxGenerator singleFlux(1);
    cont = Continuum(0.,0.,1.4e9,1.);
    singleFlux.addSpectrum(cont,x,y,wcs);
    for(int i=0;i<singleFlux.nChan();i++)
      std::cout << i << " " << singleFlux.getFlux(i) << "\n";
}
