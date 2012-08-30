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


#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <swcorrelator/MonitorFactory.h>
#include <swcorrelator/BasicMonitor.h>

#include <casa/OS/DynLib.h>        // for dynamic library loading


ASKAP_LOGGER(logger, ".monitorfactory");

namespace askap {

namespace swcorrelator {

/// @brief static registry
std::map<std::string, MonitorFactory::MonitorCreator*> MonitorFactory::theirRegistry;

/// @brief default constructor
/// @details It is declared private, so this type cannot be explicitly created.
/// All work is done via static methods
MonitorFactory::MonitorFactory() {}

/// @brief factory method
/// @details The name of the monitor is given explicitly, everything else is extracted
/// from the parset. It is done this way to be able to create multiple monitors
/// (i.e. we may want the basic monitor to co-exist with the monitoring via epics)
/// @param[in] name name of the monitor
/// @param[in] parset parset with optional parameters for the monitor (without the swcorrelator prefix)
boost::shared_ptr<IMonitor> MonitorFactory::make(const std::string &name, const LOFAR::ParameterSet &parset)
{
  if (theirRegistry.size() == 0) {
      // first call, add hard-coded monitors
      ASKAPLOG_INFO_STR(logger, "Filling the registry with pre-defined data monitors");
      addPreDefinedMonitor<BasicMonitor>();
  }
  
  std::map<std::string, MonitorFactory::MonitorCreator*>::const_iterator ci = theirRegistry.find(name);
  if (ci == theirRegistry.end()) {
      // unknown monitor, try to load from a dynamic library.
      addDLMonitor(name);
      ci = theirRegistry.find(name);
  }
  ASKAPCHECK(ci != theirRegistry.end(), "Attempted to setup an unknown data monitor "<<name);
  return ci->second(parset);
}

/// @brief templated helper method to register a hard-coded monitor
template<typename T> 
void MonitorFactory::addPreDefinedMonitor()
{
  registerMonitor(T::name(),T::setup);
}

/// @brief helper method to register a monitor
/// @param[in] name name of the monitor (key into the map)
/// @param[in] creatorFunc factory function able to create the monitor
void MonitorFactory::registerMonitor(const std::string &name, MonitorFactory::MonitorCreator *creatorFunc)
{
  ASKAPLOG_INFO_STR(logger, "      - adding '"<<name<<"' to the registry of monitors");
  ASKAPDEBUGASSERT(theirRegistry.find(name) == theirRegistry.end());
  theirRegistry[name] = creatorFunc;
}

  
/// @brief register a monitor supplied by dynamic library
/// @details The name of the monitor should be composed in the form library.setupmethod or library<setupmethod>
/// @param[in] name name of the monitor
void MonitorFactory::addDLMonitor(const std::string &name)
{
  std::string libname(toLower(name));
  const std::string::size_type pos = libname.find_first_of (".<");
  if (pos != std::string::npos) {
      libname = libname.substr (0, pos);      // only take before . or <
  }
  // Try to load the dynamic library and execute its register function.
  // Do not dlclose the library.
  ASKAPLOG_INFO_STR(logger, "Data monitor "<<name<<
                 " is not in the registry, attempting to load it dynamically from libaskap_"<<libname
                 <<".[so|dylib] and execute register_"<<libname<<"() method");
  casa::DynLib dl(libname, std::string("libaskap_"), "register_"+libname, false);
  if (dl.getHandle()) {
      // Successfully loaded. Get the creator function.
      ASKAPLOG_INFO_STR(logger, "Dynamically loaded data monitor " << name);
      // the first thing the monitor in the shared library is supposed to do is to
      // register itself. Therefore, its name will appear in the registry.
  }  
}

} // namespace swcorrelator

} // namespace askap

