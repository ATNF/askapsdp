/// @file Match output list eg. from cduchamp with known input list
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

// Package level header file
#include <askap_analysis.h>

// System includes
#include <iostream>

// ASKAPsoft includes
#include <askap/Application.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/StatReporter.h>

#include <parallelanalysis/DuchampParallel.h>
#include <preprocessing/VariableThresholder.h>
#include <casainterface/CasaInterface.h>

#include <duchamp/duchamp.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/param.hh>
#include <duchamp/Utils/Section.hh>
#include <Common/ParameterSet.h>

using namespace askap;
using namespace askap::analysis;

ASKAP_LOGGER(logger, "boxStats.log");

class BoxstatsApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            StatReporter stats;

            // This class must have scope outside the main try/catch block
            askap::askapparallel::AskapParallel comms(argc, const_cast<const char**>(argv));
            try {
                LOFAR::ParameterSet subset(config().makeSubset("BoxStats."));

		DuchampParallel parl(comms);
		duchamp::Param par;
		par.setImageFile(subset.getString("image"));
		par.setCut(subset.getFloat("snrCut"));
		par.setFlagRobustStats(subset.getBool("flagRobustStats",true));
		par.setSearchType(subset.getString("searchType","spatial"));
		std::vector<size_t> dim = analysisutilities::getCASAdimensions(par.getImageFile());
		std::vector<long> diml(dim.size());
		for(size_t i=0;i<dim.size();i++) diml[i]=dim[i];
		par.setFlagSubsection(subset.getBool("flagSubsection"));
		par.setSubsection(subset.getString("subsection",duchamp::nullSection(dim.size())));
		ASKAPCHECK(par.parseSubsections(diml)==duchamp::SUCCESS, "Could not parse subsection in param: " << par);
		parl.cube().saveParam(par);
		parl.setBaseSubsection(par.getSubsection());
		parl.setFlagVariableThreshold(true);
		parl.readData();

		VariableThresholder varThresh(comms,subset);
		if(comms.isParallel()) varThresh.setFilenames(comms);
		varThresh.initialise(parl.cube());
		varThresh.calculate();

            } catch (const askap::AskapError& x) {
                ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
                std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
                exit(1);
            } catch (const duchamp::DuchampError& x) {
                ASKAPLOG_FATAL_STR(logger, "Duchamp error in " << argv[0] << ": " << x.what());
                std::cerr << "Duchamp error in " << argv[0] << ": " << x.what() << std::endl;
                exit(1);
            } catch (const std::exception& x) {
                ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
                std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
                exit(1);
            }

            stats.logSummary();
            return 0;
        }
};

// Main function
int main(int argc, char *argv[])
{
    BoxstatsApp app;
    return app.main(argc, argv);
}
