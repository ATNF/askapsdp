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

#ifndef ASKAP_PROFILE_SINGLETON_H
#define ASKAP_PROFILE_SINGLETON_H

// own includes
#include <profile/ProfileTree.h>

// std includes
#include <map>

// boost includes
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

// casa includes
#include "casa/OS/Timer.h"

namespace askap {

/// @brief profiling singleton
/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This is the main class used to rout the calls to the appropriate tree, ensure
/// thread safety and dump statistics at the end. There supposed to be a single instance
/// of this class only.
/// @ingroup profile
class ProfileSingleton {
public:
   /// @brief default constructor
   /// @details Only the main thread is supposed to create an instance of this object.
   ProfileSingleton();

   /// @brief destructor which dumps all statistics
   /// @details For now we just write stats into log, later on we could change it to write
   /// stats into a file.
   ~ProfileSingleton(); 
   
   /// @brief entry event
   /// @details This method is supposed to be called at the start of tracking.
   /// The event is translated to the appropriate tree. This method is thread safe.
   /// @param[in] name name of the method
   void notifyEntry(const std::string &name);
   
   /// @brief exit event
   /// @details This method is supposed to be called at the end of the method.
   /// The event is translated to the appropriate tree. This method is thread safe.
   /// @param[in] name name of the method
   /// @param[in] time execution time interval
   void notifyExit(const std::string &name, const double time);
      
protected:
   
   /// @brief helper method to log profiling statistics
   /// @param[in] tree const reference to the profile tree to use
   /// @param[in] keepHierarchy if true, the hierarchy of nodes is kept and reflected by dot-separated names. If
   /// false, the hierarchy is ignored completely and all stats gathered at all levels are simply added up.   
   void logProfileStats(const ProfileTree &tree, bool keepHierarchy) const;
   
   /// @brief helper method to extract the tree for the given thread
   /// @details This method searches for a given thread in the map and inserts
   /// a new element if necessary. Locking is done for the time of the search/update.
   /// @return reference to the tree
   ProfileTree& getTree();
   
private:
   /// @brief profile tree for the main thread
   ProfileTree itsMainTree;
   
   /// @brief main thread global timer
   /// @detail We can add timers per thread later on, although the usefulness of it is not clear because we cannot
   /// detect when each child thread finishes.
   casa::Timer itsMainTimer;   
   
   /// @brief profile trees for child threads
   std::map<boost::thread::id, ProfileTree> itsThreadTrees;
   
   /// @brief synchronisation object to protect thread trees
   boost::shared_mutex itsMutex;
};

} // namespace askap

#endif // #ifndef ASKAP_PROFILE_SINGLETON_H

