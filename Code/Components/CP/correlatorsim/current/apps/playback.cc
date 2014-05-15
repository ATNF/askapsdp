/// @file playback.cc
///
/// @copyright (c) 2009 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include package level header file
#include <askap_correlatorsim.h>

// System includes
#include <unistd.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <mpi.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Application.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "simplayback/SimPlayback.h"

using namespace askap;

ASKAP_LOGGER(logger, ".main");

static std::string getNodeName(void)
{
    char name[MPI_MAX_PROCESSOR_NAME];
    int resultlen;
    MPI_Get_processor_name(name, &resultlen);
    std::string nodename(name);
    std::string::size_type idx = nodename.find_first_of('.');
    if (idx != std::string::npos) {
        // Extract just the hostname part
        nodename = nodename.substr(0, idx);
    }
    return nodename;
}

static std::string getRank(void)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::ostringstream ss;
    ss << rank;
    return ss.str();
}

class PlaybackApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            MPI_Init(&argc, &argv);

            // To aid in debugging, the logger needs to know the
            // MPI rank and nodename
            ASKAPLOG_REMOVECONTEXT("mpirank");
            ASKAPLOG_PUTCONTEXT("mpirank", getRank().c_str());
            ASKAPLOG_REMOVECONTEXT("hostname");
            ASKAPLOG_PUTCONTEXT("hostname", getNodeName().c_str());

            ASKAPLOG_INFO_STR(logger, "ASKAP Correlator Simulator (Playback) - " << ASKAP_PACKAGE_VERSION);

            int error = 0;
            try {
                askap::cp::SimPlayback pb(config());
                pb.run();
            } catch (const askap::AskapError& e) {
                ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
                std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
                error = 1;
            } catch (const std::exception& e) {
                ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
                std::cerr << "Unexpected exception in " << argv[0] << ": " << e.what() << std::endl;
                error = 1;
            }

            if (error) {
                MPI_Abort(MPI_COMM_WORLD, error);
            } else {
                MPI_Finalize();
            }

            return error;
        }
};

int main(int argc, char* argv[])
{
    PlaybackApp app;
    return app.main(argc, argv);
}
