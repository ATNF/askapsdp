/// @file 
///
/// @brief monitoring adapter sending data through registered call back methods
/// @details This class is an implementation of a general monitoring interface. It
/// computes average amplitudes, phases and fits for delays the same way as BasicMonitor does,
/// but then calls the registered call back methods and use MonitoringData class to carry the 
/// information. It is supposed to be used in the EPICS interface.
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
#include <askap_swcorrelator.h>
#include <corrinterfaces/CallBackMonitor.h>
#include <swcorrelator/BasicMonitor.h>
#include <corrinterfaces/MonitoringData.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

// casa includes
#include <casa/BasicSL.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Quanta.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MeasFrame.h>


// boost include
#include <boost/shared_ptr.hpp>

ASKAP_LOGGER(logger, ".callbackmonitor");

namespace askap {

namespace swcorrelator {

// static data members for the singleton
boost::shared_ptr<CallBackMonitor> CallBackMonitor::theirInstance;

boost::mutex CallBackMonitor::theirInstanceMutex;
//

/// @brief setup call back function
/// @details If not NULL, the given function will be called every time the new data arrive.
/// @param[in] callBackPtr pointer to the call back function or NULL to remove the callback corresponding to
/// the given same optional data
/// @param[in[ optionalData optional pointer which is then passed to call back function
/// @note the meaning of optionalData is user interpreted. It doesn't need to be a valid pointer
void CallBackMonitor::setCallBack(CallBackType callBackPtr, void* optionalData) 
{
  boost::lock_guard<boost::mutex> lock(itsRegistryMutex);
  std::map<void*, CallBackType>::iterator it = itsCallBackRegistry.find(optionalData);
  if (it != itsCallBackRegistry.end()) {
      ASKAPDEBUGASSERT(it->first == optionalData);
      if (callBackPtr == NULL) {
          // delete the entry
          itsCallBackRegistry.erase(it);
      } else {
         it->second = callBackPtr;
      }
  } else if (callBackPtr != NULL) {
     itsCallBackRegistry[optionalData] = callBackPtr;
  } else {
     ASKAPLOG_WARN_STR(logger, "An attempt to set NULL call back pointer for optionalData="<<optionalData);
  }
}

/// @brief create and configure the monitor   
/// @details
/// @return shared pointer to the monitor
boost::shared_ptr<IMonitor> CallBackMonitor::setup(const LOFAR::ParameterSet &) 
{
  ASKAPLOG_INFO_STR(logger, "Setting up EPICS-specific Data Monitor (call back)");  
  boost::lock_guard<boost::mutex> lock(theirInstanceMutex);
  if (theirInstance) {
      ASKAPLOG_INFO_STR(logger, "  - the data monitor has already been created, reusing it");
  } else {
     theirInstance.reset(new CallBackMonitor);
  }
  ASKAPDEBUGASSERT(theirInstance);
  return theirInstance;
}

/// @brief obtain the singleton
/// @details There is only one instance of this class. Although we could've had everything static, such a
/// design looks ugly. We can't make the instance static because it is created inside the software correlator
/// by a factory. This method initialises the singleton if necessary and returns a reference.
/// @return reference to the singleton
CallBackMonitor& CallBackMonitor::monitor()
{
  { 
     // block to unlock the mutex after the check
     boost::lock_guard<boost::mutex> lock(theirInstanceMutex);
     if (theirInstance) {
         return *theirInstance;
     }
  }
  setup(LOFAR::ParameterSet());
  boost::lock_guard<boost::mutex> lock(theirInstanceMutex);
  ASKAPDEBUGASSERT(theirInstance); 
  return *theirInstance;
}

    
/// @brief initialise publishing
/// @details Technically, this step is not required. But given the
/// current design of the code it seems better to give a hint on the maximum
/// possible number of antennas, beams and channels, e.g. to initialise caches.
/// @param[in] nAnt maximum number of antennas
/// @param[in] nBeam maximum number of beams
/// @param[in] nChan maximum number of channels
/// @note At the moment we envisage that this method would only be called once.
/// Technically all this information could be extracted from the parset supplied
/// in the setup method, but it seems handy to have each parameter extracted from
/// the parset at a single place only.  
void CallBackMonitor::initialise(const int nAnt, const int nBeam, const int nChan)
{
  ASKAPLOG_INFO_STR(logger, "Initialise EPICS-specific Data Monitor for nAnt="<<nAnt<<" nBeam="<<nBeam<<" and nChan="<<nChan);
}
   
/// @brief Publish one buffer of data
/// @details This method is called as soon as the new chunk of data is written out
/// @param[in] buf products buffer
/// @note the buffer is locked for the duration of execution of this method, different
/// beams are published separately
void CallBackMonitor::publish(const CorrProducts &buf)
{
  // for simplicity do all operations here. Some form of buffering can be implemented later
  MonitoringData result(buf.itsBeam);
  
  // calculate UTC
  // note, we need to specify 'ull' type for the constant as the value exceeds the capacity of long, 
  // which is assumed by default
  const uint64_t microsecondsPerDay = 86400000000ull;
  const casa::MVEpoch timeTAI(double(buf.itsBAT / microsecondsPerDay), double(buf.itsBAT % microsecondsPerDay)/double(microsecondsPerDay));
  const casa::MEpoch epoch = casa::MEpoch::Convert(casa::MEpoch(timeTAI, casa::MEpoch::Ref(casa::MEpoch::TAI)), 
                             casa::MEpoch::Ref(casa::MEpoch::UTC))();
  result.itsTime = epoch.getValue().get();                           
  //
  casa::Vector<casa::Float> delays = BasicMonitor::estimateDelays(buf.itsVisibility);
  ASKAPDEBUGASSERT(buf.itsVisibility.nrow() == result.itsDelays.size());
  ASKAPDEBUGASSERT(buf.itsVisibility.nrow() == delays.size());
  ASKAPDEBUGASSERT(buf.itsVisibility.nrow() == result.itsAmplitudes.size());
  ASKAPDEBUGASSERT(buf.itsVisibility.nrow() == result.itsPhases.size());
  ASKAPDEBUGASSERT(buf.itsVisibility.nrow() == result.itsFlags.size());
  for (casa::uInt baseline = 0; baseline < buf.itsVisibility.nrow(); ++baseline) {
       result.itsDelays[baseline] = delays[baseline]*1e9; // in nsec
       // average in frequency
       casa::Complex temp(0.,0.);
       casa::uInt counter = 0;
       for (casa::uInt chan=0; chan < buf.itsVisibility.ncolumn(); ++chan) {
            if (!buf.itsFlag(baseline,chan)) {
                temp += buf.itsVisibility(baseline,chan);
                ++counter;
            }
       }
       if (counter > 0) {
           temp /= float(counter);
       } else {
           // all flagged
           temp = 0.;
           result.itsFlags[baseline] = true;
       }
       result.itsAmplitudes[baseline] = abs(temp);
       result.itsPhases[baseline] = arg(temp)/casa::C::pi*180.; // in degrees      
  }
  // publish the result via call-backs
  boost::lock_guard<boost::mutex> lock(itsRegistryMutex);
  for (std::map<void*, CallBackType>::const_iterator ci = itsCallBackRegistry.begin(); 
              ci != itsCallBackRegistry.end(); ++ci) {
       (*ci->second)(result,ci->first);
  }  
}
  
/// @brief finilaise publishing for the current integration
/// @details This method is called when data corresponding to all beams are published.
/// It is the place for operations which do not require the lock on the buffers
/// (i.e. dumping the accumulated history to the file, etc).
void CallBackMonitor::finalise()
{
  // do nothing for now, later on we can implement buffering per beam in publish and iteration over
  // call back methods here.
}


} // namespace swcorrelator

} // namespace askap

