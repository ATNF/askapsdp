/// @file datafiller.cc
///
/// @copyright (c) 2010 CSIRO
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

// Must be included first
#include "askap_cpfiller.h"

// System includes
#include <string>
#include <stdexcept>
#include <unistd.h>

// ASKAPsoft includes
#include "askap/Application.h"
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "askap/StatReporter.h"

// Local package includes
#include "datafiller/DataFiller.h"

// Using
using namespace askap;
using namespace askap::cp::datafiller;

ASKAP_LOGGER(logger, ".main");

class DatafillerApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            int error = 0;
            try {
                ASKAPLOG_INFO_STR(logger, "ASKAP Datafiller - " << ASKAP_PACKAGE_VERSION);

                StatReporter stats;

                // Run the data filler
                DataFiller filler(config());
                filler.start();

                stats.logSummary();
            } catch (const askap::AskapError& e) {
                ASKAPLOG_ERROR_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
                std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
                error = 1;
            } catch (const std::exception& e) {
                ASKAPLOG_ERROR_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
                std::cerr << "Unexpected exception in " << argv[0] << ": " << e.what()
                    << std::endl;
                error = 1;
            }

            return error;
        }
};

int main(int argc, char *argv[])
{
    DataFillerApp app;
    return app.main(argc, argv);
}
