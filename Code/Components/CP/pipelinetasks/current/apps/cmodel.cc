/// @file cmodel.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "askap_pipelinetasks.h"

// System include
#include <string>
#include <fstream>
#include <sstream>

// ASKAPsoft includes
#include "askap/Application.h"
#include "askap/AskapError.h"
#include "askap/StatReporter.h"
#include "Common/ParameterSet.h"

// Local packages includes
#include "cmodel/MPIBasicComms.h"
#include "cmodel/CModelMaster.h"
#include "cmodel/CModelWorker.h"

// Using
using namespace askap;
using namespace askap::cp::pipelinetasks;

class CmodelApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            StatReporter stats;
            LOFAR::ParameterSet subset = config().makeSubset("Cmodel.");

            // Create the comms instance
            MPIBasicComms comms(argc, argv);

            // Instantiate and run the model creator
            if (comms.getId() == 0) {
                CModelMaster master(subset, comms);
                master.run();
            } else {
                CModelWorker worker(comms);
                worker.run();
            }

            stats.logSummary();
            return 0;
        }
};

int main(int argc, char *argv[])
{
    CmodelApp app;
    return app.main(argc, argv);
}
