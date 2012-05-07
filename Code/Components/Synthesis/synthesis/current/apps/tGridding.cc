/// @file
///
/// @breif tests for gridding run in parallel
///
/// Performs synthesis imaging from a data source, using any of a number of
/// image solvers. Can run in serial or parallel (MPI) mode.
///
/// The data are accessed from the DataSource. This is and will probably remain
/// disk based. The images are kept purely in memory until the end.
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// Package level header file
#include "askap_synthesis.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include <fitting/Params.h>
#include "askap/StatReporter.h"
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>
#include <CommandLineParser.h>
#include <askapparallel/AskapParallel.h>
#include <Common/ParameterSet.h>
#include <gridding/VisGridderFactory.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <askap/AskapUtil.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>


ASKAP_LOGGER(logger, ".cimager");

using namespace askap;
using namespace askap::synthesis;
using namespace askap::scimath;
using namespace askap::accessors;

// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, argv);

    try {
        // Ensure that CASA log messages are captured
        casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
        casa::LogSink::globalSink(globalSink);

        StatReporter stats;

        // Put everything in scope to ensure that all destructors are called
        // before the final message
        {
                    cmdlineparser::Parser parser; // a command line parser
            // command line parameter
            cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs",
                    "tgridding.in");
            // this parameter is optional
            parser.add(inputsPar, cmdlineparser::Parser::return_default);

            parser.process(argc, argv);

            const std::string parsetFile = inputsPar;

            LOFAR::ParameterSet parset(parsetFile);
            LOFAR::ParameterSet subset(parset.isDefined("Cimager.gridder") ? parset.makeSubset("Cimager.") : parset);
            
            ASKAPLOG_INFO_STR(logger, "Setting up the gridder to test and the model");
            IVisGridder::ShPtr gridder = VisGridderFactory::make(subset);
            ASKAPCHECK(gridder, "Gridder is not defined");
            scimath::Params model;
            boost::shared_ptr<scimath::Params> modelPtr(&model, utility::NullDeleter());
            SynthesisParamsHelper::setUpImages(modelPtr,subset.makeSubset("Images."));
            ASKAPLOG_INFO_STR(logger, "Model contains the following elements: "<<model);
            
            const int nCycles = subset.getInt32("ncycles", 1);
            ASKAPCHECK(nCycles > 0, "Number of iterations over the dataset is supposed to be positive, you have "<<nCycles);
            const std::string dataset = subset.getString("dataset");
            ASKAPLOG_INFO_STR(logger, "Dataset "<<dataset<<" will be used");
            accessors::TableDataSource ds(dataset, accessors::TableDataSource::MEMORY_BUFFERS, "DATA");
            accessors::IDataSelectorPtr sel=ds.createSelector();
            sel << subset;
            accessors::IDataConverterPtr conv=ds.createConverter();
            conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
            conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
            // ensure that time is counted in seconds since 0 MJD
            conv->setEpochFrame();
                    
            for (int cycle = 0; cycle < nCycles; ++cycle) {
                 ASKAPLOG_INFO_STR(logger, "-------------- 'Major cycle' number "<<(cycle + 1)<< " -----------------");
                 accessors::IDataSharedIter it=ds.createIterator(sel, conv);
                 
            }
            
        }
        stats.logSummary();
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        std::cerr << "Usage: " << argv[0] << " [-inputs parsetFile]"
                      << std::endl;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what()
                      << std::endl;
        exit(1);
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what()
                      << std::endl;
        exit(1);
    }

    return 0;
}
        