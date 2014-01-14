/// @file cimstat.cc
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>

// Package level header file
#include <askap_analysis.h>

// System includes
#include <stdexcept>
#include <iostream>

// ASKAPSoft includes
#include <askap/Application.h>
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <parallelanalysis/DuchampParallel.h>
#include <Common/ParameterSet.h>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/utils.hh>

using std::cout;
using std::endl;

using namespace askap;
using namespace askap::analysis;

ASKAP_LOGGER(logger, "");

// Main function
class CimstatApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            // This class must have scope outside the main try/catch block
            askap::askapparallel::AskapParallel comms(argc, const_cast<const char**>(argv));
            try {
                LOFAR::ParameterSet subset(config().makeSubset("Cimstat."));
		std::vector<std::string> statList = subset.getStringVector("stats");
                DuchampParallel finder(comms, subset);
                finder.readData();
                finder.gatherStats();

		if ( comms.isMaster() ){
		    ASKAPLOG_INFO_STR(logger,"Requested stats follow:");
		
		    for(std::vector<std::string>::iterator stat=statList.begin();stat<statList.end();stat++){
			std::string st=makelower(*stat);
			if(st == "mean") 
			    ASKAPLOG_INFO_STR(logger, "Mean = " << finder.cube().stats().getMean());
			else if (st == "stddev")
			    ASKAPLOG_INFO_STR(logger, "Stddev = " << finder.cube().stats().getStddev());
			else if (st == "median"){
			    if (comms.isParallel())
				ASKAPLOG_WARN_STR(logger, "Running in parallel mode, so no median value available");
			    else
				ASKAPLOG_INFO_STR(logger, "Median = " << finder.cube().stats().getMedian());
			}
			else if (st == "madfm") {
			    if (comms.isParallel())
				ASKAPLOG_WARN_STR(logger, "Running in parallel mode, so no madfm value available");
			    else
				ASKAPLOG_INFO_STR(logger, "Madfm = " << finder.cube().stats().getMadfm());
			}
			else if (st == "madfmasstddev") {
			    if (comms.isParallel())
				ASKAPLOG_WARN_STR(logger, "Running in parallel mode, so no madfm value available");
			    else
				ASKAPLOG_INFO_STR(logger, "Madfm = " << Statistics::madfmToSigma<float>(finder.cube().stats().getMadfm()));
			}
			else
			    ASKAPLOG_WARN_STR(logger, "Requested statistic '"<<*stat << "' not available");
		    }
		}

            } catch (const askap::AskapError& x) {
                ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
                std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
                exit(1);
            } catch (const std::exception& x) {
                ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
                std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
                exit(1);
            }

            return 0;
        }
};

int main(int argc, char *argv[])
{
    CimstatApp app;
    return app.main(argc, argv);
}
