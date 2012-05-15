/// @file
/// @brief profiling singleton
///
/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This is the main class used to rout the calls to the appropriate tree, ensure
/// thread safety and dump statistics at the end. There supposed to be a single instance
/// of this class only.
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

#include <profile/ProfileSingleton.h>
#include <profile/ProfileData.h>
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>

#include <string>

using namespace askap;

ASKAP_LOGGER(logger, ".ProfileSingleton");

/// @brief static singleton
boost::shared_ptr<ProfileSingleton> ProfileSingleton::theirSingleton;

/// @brief initialise singleton 
/// @details This step is essential before capture of profile information
void ProfileSingleton::start() {
  ASKAPCHECK(!theirSingleton, "ProfileSingleton::start is supposed to be called only once!");
  theirSingleton.reset(new ProfileSingleton);
}
   
/// @brief finalise singleton
/// @details We need an explicit step to be able to run destructors before logger is terminated.
void ProfileSingleton::stop() {
  ASKAPCHECK(theirSingleton, "ProfileSingleton::stop is supposed to be called after start!");
  theirSingleton.reset();
}


/// @brief default constructor
ProfileSingleton::ProfileSingleton() : itsMainThreadID(boost::this_thread::get_id())
{
   ASKAPLOG_INFO_STR(logger, "Profiling statistics will be gathered");
   itsMainTimer.mark();
}

/// @brief destructor which dumps all statistics
/// @details For now we just write stats into log, later on we could change it to write
/// stats into a file.
ProfileSingleton::~ProfileSingleton() {
  ASKAPCHECK(itsMainTree.isRootCurrent(), "Detected a mismatch between entry/exit events!");  
  itsMainTree.notifyExit(itsMainTimer.real());  
  ASKAPLOG_INFO_STR(logger, "Profiling statistics with hierarchy (main thread):");
  logProfileStats(itsMainTree, true);
  ASKAPLOG_INFO_STR(logger, "Profiling statistics without hierarchy (main thread):");
  logProfileStats(itsMainTree, false);
  
  // the following lock is not really necessary as there should be no threads doing active work at this point
  boost::lock_guard<boost::shared_mutex> lock(itsMutex);
  for (std::map<boost::thread::id, ProfileTree>::const_iterator ci = itsThreadTrees.begin(); 
       ci != itsThreadTrees.end(); ++ci) {
       ASKAPLOG_INFO_STR(logger, "Profiling statistics with hierarchy (thread "<<ci->first<<"):");
       logProfileStats(ci->second, true);
       ASKAPLOG_INFO_STR(logger, "Profiling statistics without hierarchy (thread "<<ci->first<<"):");
       logProfileStats(ci->second, false);
  }
}

/// @brief helper method to log profiling statistics
/// @param[in] tree const reference to the profile tree to use
/// @param[in] keepHierarchy if true, the hierarchy of nodes is kept and reflected by dot-separated names. If
/// false, the hierarchy is ignored completely and all stats gathered at all levels are simply added up.   
void ProfileSingleton::logProfileStats(const ProfileTree &tree, bool keepHierarchy) const
{
  std::map<std::string, ProfileData> stats;
  tree.extractStats(stats, keepHierarchy);
  if (stats.size() == 0) {
      ASKAPLOG_INFO_STR(logger, "  no statistics captured");
  } else {
      for (std::map<std::string, ProfileData>::const_iterator ci = stats.begin(); ci != stats.end(); ++ci) {
          const ProfileData &pd = ci->second;
          ASKAPLOG_INFO_STR(logger, "  "<<ci->first<<" count: "<<pd.count()<<" total: "<<pd.totalTime()<<" max: "<<
                                    pd.maxTime()<<" min: "<<pd.minTime());
      }
  }
}

/// @brief entry event
/// @details This method is supposed to be called at the start of tracking.
/// The event is translated to the appropriate tree. This method is thread safe.
/// @param[in] name name of the method
void ProfileSingleton::notifyEntry(const std::string &name)
{
  if (boost::this_thread::get_id() == itsMainThreadID) {
      // main thread, no locking
      itsMainTree.notifyEntry(name);
  } else {
      getTree().notifyEntry(name);      
  }
}
   
/// @brief exit event
/// @details This method is supposed to be called at the end of the method.
/// The event is translated to the appropriate tree. This method is thread safe.
/// @param[in] name name of the method
/// @param[in] time execution time interval
void ProfileSingleton::notifyExit(const std::string &name, const double time)
{
  if (boost::this_thread::get_id() == itsMainThreadID) {
      // main thread, no locking
      itsMainTree.notifyExit(name,time);
  } else {
      getTree().notifyExit(name,time);      
  }
}

/// @brief helper method to extract the tree for the given thread
/// @details This method searches for a given thread in the map and inserts
/// a new element if necessary. Locking is done for the time of the search/update.
/// @return reference to the tree
ProfileTree& ProfileSingleton::getTree()
{
   // all locking happens inside this method, once the element is created it is safe
   // to use its reference in a single thread.
   const boost::thread::id id = boost::this_thread::get_id();
   ASKAPDEBUGASSERT(id != itsMainThreadID);
   {
     // multiple reads are allowed
     boost::shared_lock<boost::shared_mutex> lock(itsMutex);
     std::map<boost::thread::id, ProfileTree>::iterator it = itsThreadTrees.find(id);
     if (it != itsThreadTrees.end()) {
         // element is present, no exclusive lock is necessary because the 
         // address always stays the same
         return it->second;
     }     
   }
   // the element is not found, we need an exclusive lock to create one
   boost::unique_lock<boost::shared_mutex> lock(itsMutex);
   // the following will execute a default constructor of the tree in the background
   return itsThreadTrees[id];
}



