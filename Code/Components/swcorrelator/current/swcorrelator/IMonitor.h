/// @file 
///
/// @brief generic interface for an on-the-fly monitor
/// @details Possible implementations could include dumping some history
/// into ascii files or providing monitoring via epics.
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

#ifndef ASKAP_SWCORRELATOR_I_MONITOR_H
#define ASKAP_SWCORRELATOR_I_MONITOR_H

// own includes
#include <swcorrelator/CorrProducts.h>

// std includes
#include <string>

// boost includes
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace askap {

namespace swcorrelator {

/// @brief generic interface for an on-the-fly monitor
/// @details Possible implementations could include dumping some history
/// into ascii files or providing monitoring via epics.
/// @ingroup swcorrelator
struct IMonitor : private boost::noncopyable {
  
  /// @brief shared pointer type
  typedef boost::shared_ptr<IMonitor> ShPtr;
 
  /// @brief virtual destructor to keep the compiler happy
  virtual ~IMonitor() {};
  
  /// @brief name of the monitor
  /// @return the name of the monitor
  static std::string name() { return "undefined"; }
  
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
  virtual void initialise(const int nAnt, const int nBeam, const int nChan) = 0;
    
  /// @brief Publish one buffer of data
  /// @details This method is called as soon as the new chunk of data is written out
  /// @param[in] buf products buffer
  /// @note the buffer is locked for the duration of execution of this method, different
  /// beams are published separately
  virtual void publish(const CorrProducts &buf) = 0;
  
  /// @brief finilaise publishing for the current integration
  /// @details This method is called when data corresponding to all beams are published.
  /// It is the place for operations which do not require the lock on the buffers
  /// (i.e. dumping the accumulated history to the file, etc).
  virtual void finalise() = 0;
};
  
} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_I_MONITOR_H

