/// @file 
///
/// @brief a factory class creating data monitors 
/// @details We support both built-in and dynamically loadable
/// data monitors (i.e. something which is called for every chunk of the
/// data written to the MS). The latter can be used for example to implement
/// monitoring via epics.
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

#ifndef ASKAP_SWCORRELATOR_MONITOR_FACTORY_H
#define ASKAP_SWCORRELATOR_MONITOR_FACTORY_H

// own includes
#include <swcorrelator/IMonitor.h>

// std includes
#include <string>
#include <map>

// boost includes
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

// other 3rd party
#include <Common/ParameterSet.h>

namespace askap {

namespace swcorrelator {

/// @brief a factory class creating data monitors 
/// @details We support both built-in and dynamically loadable
/// data monitors (i.e. something which is called for every chunk of the
/// data written to the MS). The latter can be used for example to implement
/// monitoring via epics.
/// @ingroup swcorrelator
class MonitorFactory : private boost::noncopyable {

  /// @brief default constructor
  /// @details It is declared private, so this type cannot be explicitly created.
  /// All work is done via static methods
  MonitorFactory();

public:   
  /// @brief signature of the factory method
  /// @details All functions creating an IMonitor objects must have this
  /// signature. Preferably, such a function should be a static method of the 
  /// appropriate monitor class.
  typedef IMonitor::ShPtr MonitorCreator(const LOFAR::ParameterSet &);
  
  /// @brief helper method to register a monitor
  /// @param[in] name name of the monitor (key into the map)
  /// @param[in] creatorFunc factory function able to create the monitor
  static void registerMonitor(const std::string &name, MonitorCreator *creatorFunc);

  /// @brief factory method
  /// @details The name of the monitor is given explicitly, everything else is extracted
  /// from the parset. It is done this way to be able to create multiple monitors
  /// (i.e. we may want the basic monitor to co-exist with the monitoring via epics)
  /// @param[in] name name of the monitor
  /// @param[in] parset parset with optional parameters for the monitor (without the swcorrelator prefix)
  static boost::shared_ptr<IMonitor> make(const std::string &name, const LOFAR::ParameterSet &parset);

protected:
  /// @brief templated helper method to register a hard-coded monitor
  template<typename T> 
  static void addPreDefinedMonitor();
  
  /// @brief register a monitor supplied by dynamic library
  /// @details The name of the monitor should be composed in the form library.setupmethod or library<setupmethod>
  /// @param[in] name name of the monitor
  static void addDLMonitor(const std::string &name);
      
private:
  
  /// @brief static registry
  static std::map<std::string, MonitorCreator*> theirRegistry;
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_MONITOR_FACTORY_H

