/// @file 
///
/// @brief test application for the real time software correlator
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

// own includes
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <corrinterfaces/CorrRunner.h>

// casa includes
#include <casa/OS/Timer.h>

// other 3rd party
#include <askapparallel/AskapParallel.h>
#include <Common/ParameterSet.h>

#include <boost/thread/thread.hpp>

ASKAP_LOGGER(logger, ".tCorrWrapper");

// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, argv);
    
    try {
       casa::Timer timer;
       timer.mark();
                   
       askap::swcorrelator::CorrRunner runner;
       ASKAPLOG_INFO_STR(logger,  "swcorrelator wrapper: running="<<runner.isRunning()<<" status="<<runner.statusMsg());
       runner.start("apps/test.in");      
       ASKAPLOG_INFO_STR(logger,  "swcorrelator wrapper: running="<<runner.isRunning()<<" status="<<runner.statusMsg());
       boost::this_thread::sleep(boost::posix_time::seconds(220));          
       ASKAPLOG_INFO_STR(logger,  "swcorrelator wrapper: running="<<runner.isRunning()<<" status="<<runner.statusMsg());
       runner.stop();      
       ASKAPLOG_INFO_STR(logger,  "stop requested: running="<<runner.isRunning()<<" status="<<runner.statusMsg());
       boost::this_thread::sleep(boost::posix_time::seconds(2));          
       ASKAPLOG_INFO_STR(logger,  "swcorrelator wrapper: running="<<runner.isRunning()<<" status="<<runner.statusMsg());
       boost::this_thread::sleep(boost::posix_time::seconds(10));          
       ASKAPLOG_INFO_STR(logger,  "swcorrelator wrapper: running="<<runner.isRunning()<<" status="<<runner.statusMsg());
       
       ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                          << " real:   " << timer.real());
        ///==============================================================================
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        return 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        return 1;
    }

    ASKAPLOG_INFO_STR(logger,  "tCorrWrapper existing...");
    return 0;
}
