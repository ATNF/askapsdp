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

#include <profile/AskapProfiler.h>
#include <profile/ProfileSingleton.h>

using namespace askap;


/// @brief constructor, logs entry event
/// @param[in] name name of the current method or block
Profiler::Profiler(const std::string &name) : itsName(name) 
{ 
  if (ProfileSingleton::get()) {
      ProfileSingleton::get()->notifyEntry(name); 
      itsTimer.mark();
  }
}
   
/// @brief destructor, logs exit event
Profiler::~Profiler() 
{ 
  if (ProfileSingleton::get()) {
      ProfileSingleton::get()->notifyExit(itsName, itsTimer.real());
  } 
}

     
