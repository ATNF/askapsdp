/// @file 
///
/// @brief a collection of data monitors
/// @details This class is just a container of data monitors. It implements basic calls
/// of the IMonitor interface and translates them to each monitor held in the container.
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

#include <askap_swcorrelator.h>
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>

#include <swcorrelator/DataMonitors.h>
#include <swcorrelator/MonitorFactory.h>

ASKAP_LOGGER(logger, ".datamonitors");

namespace askap {

namespace swcorrelator {

/// @brief constructor, creates monitors using the factory and adds them to the container
/// @param[in] parset parset file used (without the swcorrelator prefix)
/// @note The list of monitors to create is specified by the "monitors" keyword.
DataMonitors::DataMonitors(const LOFAR::ParameterSet &parset)
{
  const std::vector<std::string> monitors = parset.getStringVector("monitors",std::vector<std::string>(1,"basic"));
  if (monitors.size() == 0) {
     ASKAPLOG_INFO_STR(logger, "No on-the-fly data monitors will be created");
  } else {
     itsMonitors.reserve(monitors.size());
     ASKAPLOG_INFO_STR(logger, "Setting up data monitors from the list: "<<monitors);
     for (std::vector<std::string>::const_iterator ci = monitors.begin(); ci != monitors.end(); ++ci) {
          const boost::shared_ptr<IMonitor> curMonitor = MonitorFactory::make(*ci, parset);
          ASKAPCHECK(curMonitor, "Failed to create data monitor name = `"<<*ci<<"`");
          itsMonitors.push_back(curMonitor);
     }
  }
}

// @brief destructor, added to assist synchronisation
DataMonitors::~DataMonitors()
{
  boost::lock_guard<boost::mutex> lock(itsMonitorsMutex);
  for (std::vector<boost::shared_ptr<IMonitor> >::iterator it = itsMonitors.begin();
       it != itsMonitors.end(); ++it) {
       ASKAPDEBUGASSERT(*it);
       it->reset();
  }
  itsMonitors.resize(0);
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
void DataMonitors::initialise(const int nAnt, const int nBeam, const int nChan)
{
  boost::lock_guard<boost::mutex> lock(itsMonitorsMutex);
  for (std::vector<boost::shared_ptr<IMonitor> >::const_iterator ci = itsMonitors.begin();
       ci != itsMonitors.end(); ++ci) {
       ASKAPDEBUGASSERT(*ci);
       (*ci)->initialise(nAnt,nBeam,nChan);
  }
}
    
/// @brief Publish one buffer of data
/// @details This method is called as soon as the new chunk of data is written out
/// @param[in] buf products buffer
/// @note the buffer is locked for the duration of execution of this method, different
/// beams are published separately
void DataMonitors::publish(const CorrProducts &buf)
{
  boost::lock_guard<boost::mutex> lock(itsMonitorsMutex);
  for (std::vector<boost::shared_ptr<IMonitor> >::const_iterator ci = itsMonitors.begin();
       ci != itsMonitors.end(); ++ci) {
       ASKAPDEBUGASSERT(*ci);
       (*ci)->publish(buf);
  }
}
  
/// @brief finilaise publishing for the current integration
/// @details This method is called when data corresponding to all beams are published.
/// It is the place for operations which do not require the lock on the buffers
/// (i.e. dumping the accumulated history to the file, etc).
void DataMonitors::finalise()
{
  boost::lock_guard<boost::mutex> lock(itsMonitorsMutex);
  for (std::vector<boost::shared_ptr<IMonitor> >::const_iterator ci = itsMonitors.begin();
       ci != itsMonitors.end(); ++ci) {
       ASKAPDEBUGASSERT(*ci);
       (*ci)->finalise();
  }
}

} // namespace swcorrelator

} // namespace askap

