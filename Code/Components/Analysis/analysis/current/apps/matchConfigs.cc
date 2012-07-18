#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>

#include <patternmatching/Matcher.h>
#include <patternmatching/GrothTriangles.h>
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

        if (!fg.is_open()) ASKAPTHROW(AskapError, "Could not open " << guptaFile);

        if (!fd.is_open()) ASKAPTHROW(AskapError, "Could not open " << deboerFile);

        double x, y;
        int id = 1;

        while (fg >> x >> y, !fg.eof()) {
            std::stringstream ss;
            ss << id++;
            Point p(x, y, 1., ss.str(), 0, 0, 0,0,0);
            gupta.push_back(p);
        }

        id = 1;

        while (fd >> x >> y, !fd.eof()) {
            std::stringstream ss;
            ss << id++;
            Point p(x, y, 1., ss.str(), 0, 0, 0,0,0);
            deboer.push_back(p);
        }

        ASKAPLOG_INFO_STR(logger, "Sizes of lists: gupta=" << gupta.size() << ", deBoer=" << deboer.size());

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
