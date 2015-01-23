/// @file matchConfigs.cc
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

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>

#include <patternmatching/Matcher.h>
#include <Common/ParameterSet.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace askap;
using namespace askap::analysis;
using namespace askap::analysis::matching;

ASKAP_LOGGER(logger, ".matchConfigs.log");

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
        // Ensure that CASA log messages are captured
        casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
        casa::LogSink::globalSink(globalSink);

        std::string guptaFile, deboerFile;

        if (argc == 3) {
            guptaFile = argv[1];
            deboerFile = argv[2];
        } else {
            guptaFile = "/Users/whi550/PROJECTS/ASKAP/Configuration/A27CR3P6-input.dat";
            deboerFile = "/Users/whi550/PROJECTS/ASKAP/Configuration/newset_jun10.dat";
        }

        std::vector<Point> gupta, deboer;
        std::ifstream fg(guptaFile.c_str());
        std::ifstream fd(deboerFile.c_str());

        if (!fg.is_open()) {
            ASKAPTHROW(AskapError, "Could not open " << guptaFile);
        }

        if (!fd.is_open()) {
            ASKAPTHROW(AskapError, "Could not open " << deboerFile);
        }

        double x, y;
        int id = 1;

        while (fg >> x >> y, !fg.eof()) {
            std::stringstream ss;
            ss << id++;
            //            Point p(x, y, 1., ss.str(), 0, 0, 0,0,0);
            Point p(x, y, 1., ss.str());
            gupta.push_back(p);
        }

        id = 1;

        while (fd >> x >> y, !fd.eof()) {
            std::stringstream ss;
            ss << id++;
            //            Point p(x, y, 1., ss.str(), 0, 0, 0,0,0);
            Point p(x, y, 1., ss.str());
            deboer.push_back(p);
        }

        ASKAPLOG_INFO_STR(logger, "Sizes of lists: gupta=" << gupta.size() <<
                          ", deBoer=" << deboer.size());

        LOFAR::ParameterSet nullset;
        Matcher matcher(nullset);
        matcher.setRefList(gupta);
        matcher.setSrcList(deboer);
        matcher.setTriangleLists();
        matcher.findMatches();
        matcher.findOffsets();
        matcher.addNewMatches();
        matcher.outputLists();

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
