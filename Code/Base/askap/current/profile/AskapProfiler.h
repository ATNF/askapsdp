/// @file
/// @brief profiling utilities
///
/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This is the main include file with macro definition and configure methods.
///
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
///

#ifndef ASKAP_ASKAP_PROFILER_H
#define ASKAP_ASKAP_PROFILER_H

#include <profile/ProfileSingleton.h>

// casa includes
#include "casa/OS/Timer.h"

// std includes
#include <string>

namespace askap {

#define ASKAPTRACE(name) \
    askap::Profiler askapProfilerEventGuard(name);

#ifdef ASKAP_DEBUG
#define ASKAPDEBUGTRACE(name) ASKAPTRACE(name)
#else
#define ASKAPDEBUGTRACE(name)
#endif

/// @brief profiler class used as a guard for entry/exit events
/// @details Instantiate this class to trace a given method or a block of code.
/// @note The subsystem needs initialisation before profiling can be done.
/// @ingroup profile
struct Profiler {
   /// @brief constructor, logs entry event
   /// @param[in] name name of the current method or block
   inline Profiler(const std::string &name) : itsName(name) { profiler().notifyEntry(name); itsTimer.mark();}
   
   /// @brief destructor, logs exit event
   inline ~Profiler() { profiler().notifyExit(itsName, itsTimer.real()); }

   // static methods working with the singleton
   
   /// @brief configure profiling
   /// @details This method is supposed to be called before any event calls
   static void configureProfiling();
   
   /// @return reference to the singleton
   static ProfileSingleton& profiler();

private:
   /// @brief timer to time a particular execution
   casa::Timer itsTimer;
   
   /// @brief unique name of the current method or block
   const std::string itsName;
   
   /// @brief singleton doing all dirty job
   static boost::shared_ptr<ProfileSingleton> theirSingleton;
};
} // namespace askap


#endif // #ifndef ASKAP_ASKAP_PROFILER_H

