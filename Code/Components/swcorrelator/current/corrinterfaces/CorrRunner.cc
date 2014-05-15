/// @file 
///
/// @brief class which can run the correlator
/// @details This class is analogous to the main method of the stand alone correlator 
/// application. It can run the correlator, get monitoring data and stop when necessary.
/// The primary goal for this interface is to run the software correlator from the epics
/// CA server.
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

#include <corrinterfaces/CorrRunner.h>
#include <corrinterfaces/CorrRunnerThread.h>
#include <askap/AskapUtil.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <swcorrelator/MonitorFactory.h>
#include <corrinterfaces/CallBackMonitor.h>

// boost includes
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

// casa includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MeasFrame.h>

ASKAP_LOGGER(logger, ".corrinterfaces");



namespace askap {

namespace swcorrelator {

/// @brief default constructor
CorrRunner::CorrRunner() : itsIsRunning(false), itsStatus("UNINITIALISED") 
{
  // create a monitor we don't use just to ensure that factory is initialised prior to
  // messing around with it
  MonitorFactory::make("basic", LOFAR::ParameterSet());
  // register the singleton with the factory, so it can be created by the software correlator
  MonitorFactory::addPreDefinedMonitor<CallBackMonitor>();
  // a work-around for casacore's lack of thread-safety
  // trigger a dummy measures calculation to get measures set up their caches in the main thread and avoid race condition
  const casa::MVEpoch junk(55e9);
  casa::MEpoch::Convert(casa::MEpoch(junk, casa::MEpoch::Ref(casa::MEpoch::TAI)), 
                             casa::MEpoch::Ref(casa::MEpoch::UTC))();
}

/// @brief setup call back function
/// @details If not NULL, the given function will be called every time the new data arrive.
/// @param[in] callBackPtr pointer to the call back function
/// @param[in[ optionalData optional pointer which is then passed to call back function
/// @note the meaning of optionalData is user interpreted. It doesn't need to be a valid pointer
void CorrRunner::setCallBack(CorrRunner::CallBackType callBackPtr, void *optionalData)
{
  CallBackMonitor::monitor().setCallBack(callBackPtr,optionalData);
}

/// @brief start the correlator
/// @details This method starts all required threads and intialises the correlator using
/// the parset.
/// @param[in] parset parset with input parameters
void CorrRunner::start(const LOFAR::ParameterSet &parset)
{
  if (isRunning()) {
     ASKAPLOG_WARN_STR(logger, "start requested while the software correlator is already running");
  } else {
     boost::shared_ptr<CorrRunner> thisPtr(this, utility::NullDeleter());
     ASKAPDEBUGASSERT(thisPtr);
     boost::shared_ptr<LOFAR::ParameterSet> parsetPtr(new LOFAR::ParameterSet(parset.makeSubset("swcorrelator.")));
     ASKAPDEBUGASSERT(parsetPtr);
     // patch the monitor setup in the parset here     
     if (parsetPtr->isDefined("monitors")) {
         ASKAPLOG_WARN_STR(logger, "Multiple monitors are not yet supported via the wrapper");
         parsetPtr->replace("monitors","callback");
     } else {
        parsetPtr->add("monitors","callback");
     }
     //
     itsCorrelatorThread = boost::thread(CorrRunnerThread(thisPtr, parsetPtr));
  }
}
  
/// @brief start the correlator
/// @details This version reads the parset from the given file.
/// @param[in] fname parset file name
void CorrRunner::start(const std::string &fname)
{
  try {
    const LOFAR::ParameterSet parset(fname);
    start(parset);
  } catch (const std::exception &ex) {
    setStatus(false, std::string("ERROR: ") + ex.what()); 
  } catch (...) {
    setStatus(false, "ERROR: unexpected exception"); 
  }
}
   
/// @brief stop the correlator
/// @details This method can be called at any time to request a stop. The correlator
/// finishes processing of the current cycle and gracefully shuts down closing the MS.
/// @note This method must be called at the end to avoid corruption of the MS. 
void CorrRunner::stop()
{
  ASKAPLOG_INFO_STR(logger, "about to request the software correlator to stop");
  CorrRunnerThread::stop();
}

/// @brief check whether the correlator is running
/// @details If it is not, the data in the data fields are not valid and all flags are set
/// to true. Note, this method is thread safe and can be called asynchronously
/// @return true if the correlator is running, false otherwise
bool CorrRunner::isRunning() const 
{
  boost::lock_guard<boost::mutex> lock(itsStatusMutex);
  return itsIsRunning;
}

/// @brief obtain the status of error message
/// @details When the correlator stops due to exception, the error message is available via
/// this method. Note, this method is thread safe and can be called asynchronously
std::string CorrRunner::statusMsg() const
{
  return itsStatus;
}

/// @brief set status message
/// @param[in] running true, if the correlator is running
/// @param[in] msg status/error message
void CorrRunner::setStatus(const bool running, const std::string &msg)
{
  boost::lock_guard<boost::mutex> lock(itsStatusMutex);
  itsIsRunning = running;
  itsStatus = msg;
}

/// @brief default destructor
CorrRunner::~CorrRunner()
{
  itsCorrelatorThread.join();
  if (isRunning()) {
     ASKAPLOG_WARN_STR(logger, "The software correlator seems to be still running in the CorrRunner destructor!");
  }
  // just to ensure the thread finishes properly
  CorrRunnerThread::stop();
  itsCorrelatorThread.join();
}


} // namespace swcorrelator

} // namespace askap

