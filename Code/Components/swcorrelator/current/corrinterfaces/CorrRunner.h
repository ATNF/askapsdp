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

#ifndef ASKAP_CORRINTERFACES_CORR_RUNNER_H
#define ASKAP_CORRINTERFACES_CORR_RUNNER_H

// own includes
#include <corrinterfaces/MonitoringData.h>

// std includes
#include <string>

// boost includes
#include <boost/utility.hpp>
#include <boost/thread/thread.hpp>

// other 3rd party
#include <Common/ParameterSet.h>

namespace askap {

namespace swcorrelator {

/// @brief class which can run the correlator
/// @details This class is analogous to the main method of the stand alone correlator 
/// application. It can run the correlator, get monitoring data and stop when necessary.
/// The primary goal for this interface is to run the software correlator from the epics
/// CA server.
/// @ingroup corrinterfaces
struct CorrRunner : private boost::noncopyable {
   
  /// @brief call back function type
  typedef void (*CallBackType)(const MonitoringData&, void* optionalData); 
   
  /// @brief default constructor
  CorrRunner(); // doesn't throw
  
  /// @brief setup call back function
  /// @details If not NULL, the given function will be called every time the new data arrive.
  /// @param[in] callBackPtr pointer to the call back function
  /// @param[in[ optionalData optional pointer which is then passed to call back function
  /// @note the meaning of optionalData is user interpreted. It doesn't need to be a valid pointer
  void setCallBack(CallBackType callBackPtr = NULL, void* optionalData = NULL);
   
  /// @brief start the correlator
  /// @details This method starts all required threads and intialises the correlator using
  /// the parset.
  /// @param[in] parset parset with input parameters
  void start(const LOFAR::ParameterSet &parset);  // doesn't throw
  
  /// @brief start the correlator
  /// @details This version reads the parset from the given file.
  /// @param[in] fname parset file name
  void start(const std::string &fname); // doesn't throw
   
  /// @brief stop the correlator
  /// @details This method can be called at any time to request a stop. The correlator
  /// finishes processing of the current cycle and gracefully shuts down closing the MS.
  /// @note This method must be called at the end to avoid corruption of the MS. 
  static void stop(); // doesn't throw
   
  // check the status
  
  /// @brief check whether the correlator is running
  /// @details If it is not, the data in the data fields are not valid and all flags are set
  /// to true. Note, this method is thread safe and can be called asynchronously
  /// @return true if the correlator is running, false otherwise
  bool isRunning() const; // doesn't throw
  
  /// @brief obtain the status of error message
  /// @details When the correlator stops due to exception, the error message is available via
  /// this method. Note, this method is thread safe and can be called asynchronously
  std::string statusMsg() const; // doesn't throw
  
  /// @brief set status message
  /// @param[in] running true, if the correlator is running
  /// @param[in] msg status/error message
  void setStatus(const bool running, const std::string &msg = "OK"); // doesn't throw

private:  
  // current execution status

  /// @brief true if the correlator is running, false otherwise
  bool itsIsRunning;

  /// @brief error or status message
  std::string itsStatus;

  /// @brief mutex to protect status message and the running state
  mutable boost::mutex itsStatusMutex;
 
  /// @brief main correlator thread  
  /// @details This class is executed in the main epics thread. The methods like start and stop
  /// return the control without waiting. The correlator is executed in a separate thread (where
  /// it spawns its own child threads)
  boost::thread theirCorrelatorThread; 
  
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_CORRINTERFACES_CORR_RUNNER_H

