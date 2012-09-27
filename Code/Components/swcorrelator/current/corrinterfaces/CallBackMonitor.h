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

#ifndef ASKAP_CORRINTERFACES_CALL_BACK_MONITOR_H
#define ASKAP_CORRINTERFACES_CALL_BACK_MONITOR_H

// own includes
#include <swcorrelator/IMonitor.h>
#include <corrinterfaces/MonitoringData.h>
#include <corrinterfaces/CorrRunner.h>

// casa includes
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// std includes
#include <map>

// other 3rd party
#include <Common/ParameterSet.h>

namespace askap {

namespace swcorrelator {

/// @brief monitoring adapter sending data through registered call back methods
/// @details This class is an implementation of a general monitoring interface. It
/// computes average amplitudes, phases and fits for delays the same way as BasicMonitor does,
/// but then calls the registered call back methods and use MonitoringData class to carry the 
/// information. It is supposed to be used in the EPICS interface.
/// @ingroup corrinterfaces
class CallBackMonitor : public IMonitor {
public:
  /// @brief signature of the call back method
  typedef CorrRunner::CallBackType CallBackType;
  
  /// @brief setup call back function
  /// @details If not NULL, the given function will be called every time the new data arrive.
  /// @param[in] callBackPtr pointer to the call back function or NULL to remove the callback corresponding to
  /// the given same optional data
  /// @param[in[ optionalData optional pointer which is then passed to call back function
  /// @note the meaning of optionalData is user interpreted. It doesn't need to be a valid pointer
  void setCallBack(CallBackType callBackPtr = NULL, void* optionalData = NULL);

  /// @brief obtain the singleton
  /// @details There is only one instance of this class. Although we could've had everything static, such a
  /// design looks ugly. We can't make the instance static because it is created inside the software correlator
  /// by a factory. This method initialises the singleton if necessary and returns a reference.
  /// @return reference to the singleton
  static CallBackMonitor& monitor();
  
  /// @brief create and configure the monitor   
  /// @details
  /// @param[in] parset parset with parameters (without the swcorrelator prefix)
  /// @return shared pointer to the monitor
  static boost::shared_ptr<IMonitor> setup(const LOFAR::ParameterSet &parset);
  
  /// @brief name of the monitor
  /// @return the name of the monitor
  static std::string name() { return "callback"; }
  
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
  virtual void initialise(const int nAnt, const int nBeam, const int nChan);
   
  /// @brief Publish one buffer of data
  /// @details This method is called as soon as the new chunk of data is written out
  /// @param[in] buf products buffer
  /// @note the buffer is locked for the duration of execution of this method, different
  /// beams are published separately
  virtual void publish(const CorrProducts &buf);
  
  /// @brief finilaise publishing for the current integration
  /// @details This method is called when data corresponding to all beams are published.
  /// It is the place for operations which do not require the lock on the buffers
  /// (i.e. dumping the accumulated history to the file, etc).
  virtual void finalise();

private:  

  /// @brief call back registry
  std::map<void*, CallBackType> itsCallBackRegistry;
  
  /// @brief mutex protecting the registry
  mutable boost::mutex itsRegistryMutex;
  
  // statics controlling the lifecycle of the singleton
  
  /// @brief shared pointer to call back monitor
  /// @details This monitor is a singleton, there is only one instance possible. 
  /// Alternatively, we could've made all data members static, but it seems to be better
  /// to minimise the number of statics and use this approach instead. Only the first
  /// setup call initialises the pointer (the parset is ignored anyway). All subsequent
  /// calls, if any, got the same pointer
  static boost::shared_ptr<CallBackMonitor> theirInstance;
  
  /// @brief mutex to protect shared pointer 
  /// @details the setup method can be called asynchronously from the parallel thread
  static boost::mutex theirInstanceMutex;
};

} // namespace swcorrelator

} // namespae askap

#endif // #ifndef ASKAP_CORRINTERFACES_CALL_BACK_MONITOR_H

