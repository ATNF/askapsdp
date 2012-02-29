/// @file : testing ways to access Measurement Sets and related information
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
#include <askap_analysis.h>

#include <parallelanalysis/DuchampParallel.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>

#include <string>
#include <iostream>

#include <duchamp/duchamp.hh>
#include <duchamp/fitsHeader.hh>

#include <wcslib/wcs.h>

using namespace askap;
using namespace askap::analysis;

ASKAP_LOGGER(logger, "tSpectralWCS.log");

int main(int argc, const char *argv[])
{
    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, argv);
    try {
      std::string imageName,outfile;

        if (argc == 1) imageName = "";
        else imageName = argv[1];
	if( argc < 3) outfile="spec.dat";
	else outfile = argv[2];
	std::ofstream out(outfile.c_str());

        LOFAR::ParameterSet parset;
	parset.replace("image",imageName);
	parset.replace("verbose","true");

	DuchampParallel duchamp(comms, parset);
	duchamp.getMetadata();
	double zsize = duchamp.cube().getDimZ();
	double xpos = duchamp.cube().getDimX()/2;
	double ypos = duchamp.cube().getDimX()/2;

	ASKAPLOG_DEBUG_STR(logger, "spectral units = " << duchamp.cube().header().getSpectralUnits()
			   << "   spectral desc = " << duchamp.cube().header().getSpectralDescription());
	//	wcsprt(duchamp.cube().header().getWCS());
	for (double z=0;z<zsize;z+=1.){
	  double vel = duchamp.cube().header().pixToVel(xpos,ypos,z);
	  
	    double deltaVel,zup=z+1,zdn=z-1;
	    if(z==0) 
	      deltaVel =  duchamp.cube().header().pixToVel(xpos,ypos,zup) -  duchamp.cube().header().pixToVel(xpos,ypos,z);
	    else if(z==(zsize-1)) 
	      deltaVel =  duchamp.cube().header().pixToVel(xpos,ypos,z) -  duchamp.cube().header().pixToVel(xpos,ypos,zdn);
	    else 
	      deltaVel =  (duchamp.cube().header().pixToVel(xpos,ypos,zup) - duchamp.cube().header().pixToVel(xpos,ypos,zdn))/2.;

	    out << z << " " << vel << " " << deltaVel << "\n";
	}
	out.close();


    } catch (askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (const duchamp::DuchampError& x) {
        ASKAPLOG_FATAL_STR(logger, "Duchamp error in " << argv[0] << ": " << x.what());
        std::cerr << "Duchamp error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    exit(0);
}
