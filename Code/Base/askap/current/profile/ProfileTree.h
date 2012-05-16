/// @file
/// @brief tree with profile information
///
/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This class represent a tree of the method calls. Every call corresponds to a node
/// which may optionally have other branches corresponding to the calls inside the given
/// method. Each node has a name, profile data, a map of optional lower level nodes and
/// a debug level (an integer to be able to filter out the required info easily). The tree
/// has a cursor pointing to the current node which can be updated. In the multi-threaded environment
/// only single thread can manipulate the cursor and update the nodes.
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

#ifndef ASKAP_PROFILE_TREE_H
#define ASKAP_PROFILE_TREE_H

// own includes
#include <profile/ProfileNode.h>
#include <profile/ProfileData.h>
#include <askap/AskapUtil.h>

// boost includes
#include <boost/shared_ptr.hpp>

// std includes
#include <string>
#include <map>

namespace askap {

/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This class represent a tree of the method calls. Every call corresponds to a node
/// which may optionally have other branches corresponding to the calls inside the given
/// method. Each node has a name, profile data, a map of optional lower level nodes and
/// a debug level (an integer to be able to filter out the required info easily). The tree
/// has a cursor pointing to the current node which can be updated. In the multi-threaded environment
/// only single thread can manipulate the cursor and update the nodes.
/// @ingroup profile
class ProfileTree {
public:
   /// @brief default constructor, creates a root node
   ProfileTree();
   
   /// @brief copy constructor
   /// @details We do not allow to copy profile tree which has accumulated some data.
   /// This is done to avoid referencing issues when shared pointers are copied.
   /// However, we need a copy constructor to be able to keep thread trees in the map.
   /// @param[in] other another instance of the tree
   ProfileTree(const ProfileTree &other);
   
   /// @brief assignment operator
   /// @details throws an exception if called
   /// @param[in] other another instance of the tree
   ProfileTree& operator=(const ProfileTree &other);
   
   /// @brief checks that the root node is current
   /// @return true if the root node is current
   bool isRootCurrent() const;
   
   /// @brief entry event
   /// @details This method is supposed to be called at the start of tracking.
   /// It creates an appropriate child if necessary and moves the cursor there.
   /// @param[in] name name of the method
   void notifyEntry(const std::string &name);
   
   /// @brief exit event
   /// @details This method is supposed to be called upon the exit of the method being tracked.
   /// It logs the statistics and moves the cursor a level up. An exception is thrown if
   /// one tries to move the cursor above the root or notifyExit doesn't have a matching notifyEntry call.
   /// @param[in] name name of the method for cross-check
   /// @param[in] time execution time
   void notifyExit(const std::string &name, const double time);
   
   /// @brief final exit event
   /// @details This method can be called only once to log the total time of execution. An exception is
   /// thrown if it is called more than once or if the cursor is not in the top position (it supposed to be
   /// at the top position at the end of the execution when all traceable methods concluded). 
   /// @param[in] time total execution time
   /// @note There is no requirement to always call this method at the end. However, if it is not done, the
   /// execution time will be zero for the root node.
   void notifyExit(const double time);
      
   // need methods to transverse the tree to extract statistics (ideally some kind of an iterator)   
   
   /// @brief extract statistics
   /// @details This method builds a map with statistics for the whole tree. The hierarchy of nodes is
   /// represented by dot-separated names used as the map key. This is the default behavior, but alternatively
   /// the hierarchy can be ignored and all statistics can be added up to get a global pie-chart.
   /// @param[in] stats map to add statistics to
   /// @param[in] keepHierarchy if true, the hierarchy of nodes is kept and reflected by dot-separated names. If
   /// false, the hierarchy is ignored completely and all stats gathered at all levels are simply added up.
   /// @param[in] leafsOnly if true, only leaf nodes are included in the map (i.e. the lowest level in every branch)
   /// @note The old content of the map is not removed, extracted statistics are just added to the given map.
   void extractStats(std::map<std::string, ProfileData> &stats, bool doHierarchy = true, bool leafsOnly = false) const;
   
protected:
   /// @brief helper method to extract statistics for a given node
   /// @details This method adds statistics to the map for a given node and all its children.
   /// The name is prefixed by dot-separated parent name (multiple levels of hierarchy are allowed).
   /// Alternatively the hierarchy can be ignored and all statistics can be added up to get a global pie-chart.
   /// This method calls itself recursively to process child nodes. 
   /// @param[in] stats map to update
   /// @param[in] prefix name prefix to be added to all node names
   /// @param[in] node shared pointer to node to work with
   /// @param[in] keepHierarchy if true, the hierarchy of nodes is kept and reflected by dot-separated names. If
   /// false, the hierarchy is ignored completely and all stats gathered at all levels are simply added up.
   /// @param[in] leafsOnly if true, only leaf nodes are included in the map (i.e. the lowest level in every branch)
   static void extractStats(std::map<std::string, ProfileData> &stats, const std::string &prefix, 
                     const boost::shared_ptr<ProfileNode> &node, bool doHierarchy, bool leafsOnly);
   
private:
   /// @brief root node of the tree
   ProfileNode itsRootNode;
   /// @brief current node pointed by the cursor
   boost::shared_ptr<ProfileNode> itsCurrentNode;
};

} // namespace askap

#endif // #ifndef ASKAP_PROFILE_TREE_H

