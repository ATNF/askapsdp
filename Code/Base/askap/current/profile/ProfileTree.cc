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

#include <profile/ProfileTree.h>
#include <profile/ProfileData.h>

#include <askap/AskapError.h>

using namespace askap;

/// @brief default constructor, creates a root node
ProfileTree::ProfileTree() : itsRootNode("root"),
    itsCurrentNode(&itsRootNode, utility::NullDeleter()) {}


/// @brief checks that the root node is current
/// @return true if the root node is current
bool ProfileTree::isRootCurrent() const
{
  ASKAPDEBUGASSERT(itsCurrentNode);
  return !itsCurrentNode->parent();
}

/// @brief entry event
/// @details This method is supposed to be called at the start of tracking.
/// It creates an appropriate child if necessary and moves the cursor there.
/// @param[in] name name of the method
void ProfileTree::notifyEntry(const std::string &name) 
{
  ASKAPDEBUGASSERT(itsCurrentNode);
  itsCurrentNode = itsCurrentNode->child(name);
}
   
/// @brief exit event
/// @details This method is supposed to be called upon the exit of the method being tracked.
/// It logs the statistics and moves the cursor a level up. An exception is thrown if
/// one tries to move the cursor above the root or notifyExit doesn't have a matching notifyEntry call.
/// @param[in] name name of the method for cross-check
/// @param[in] time execution time
void ProfileTree::notifyExit(const std::string &name, const double time) 
{
  ASKAPDEBUGASSERT(itsCurrentNode);
  ASKAPCHECK(itsCurrentNode->parent(), "An attempt to exit from the root node!");
  ASKAPCHECK(itsCurrentNode->name() == name, "Name mismatch in the tree structure, expected "<<itsCurrentNode->name()<<" received "<<name
             <<", entry/exit events don't match!");
  itsCurrentNode->data().add(time);
  itsCurrentNode = itsCurrentNode->parent();
}

/// @brief final exit event
/// @details This method can be called only once to log the total time of execution. An exception is
/// thrown if it is called more than once or if the cursor is not in the top position (it supposed to be
/// at the top position at the end of the execution when all traceable methods concluded). 
/// @param[in] time total execution time
/// @note There is no requirement to always call this method at the end. However, if it is not done, the
/// execution time will be zero for the root node.
void ProfileTree::notifyExit(const double time)
{
  ASKAPCHECK(isRootCurrent(), "An attempt to call the final ProfileTree::notifyExit with the cursor not at the top position. "
                              "Most likley, entry/exit events are not properly paired");
  ASKAPCHECK(itsRootNode.data().count() == 0, "An attempt to call the final ProfileTree::notifyExit more than once!");
  itsRootNode.data().add(time);
}


/// @brief extract statistics
/// @details This method builds a map with statistics for the whole tree. The hierarchy of nodes is
/// represented by dot-separated names used as the map key. This is the default behavior, but alternatively
/// the hierarchy can be ignored and all statistics can be added up to get a global pie-chart.
/// @param[in] stats map to add statistics to
/// @param[in] keepHierarchy if true, the hierarchy of nodes is kept and reflected by dot-separated names. If
/// false, the hierarchy is ignored completely and all stats gathered at all levels are simply added up.
/// @param[in] leavesOnly if true, only leaf nodes are included in the map (i.e. the lowest level in every branch)
/// @note The old content of the map is not removed, extracted statistics are just added to the given map.
void ProfileTree::extractStats(std::map<std::string, ProfileData> &stats, bool doHierarchy, bool leavesOnly) const
{ 
  boost::shared_ptr<const ProfileNode> root(&itsRootNode, utility::NullDeleter());
  // use "::" prefix to avoid accidental merge of the final execution statistics if there is another method called root
  extractStats(stats, doHierarchy || leavesOnly ? "" : "::", boost::const_pointer_cast<ProfileNode>(root), doHierarchy, leavesOnly);
}

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
/// @param[in] leavesOnly if true, only leaf nodes are included in the map (i.e. the lowest level in every branch)
void ProfileTree::extractStats(std::map<std::string, ProfileData> &stats, const std::string &prefix, 
                  const boost::shared_ptr<ProfileNode> &node, bool doHierarchy, bool leavesOnly)
{
  ASKAPDEBUGASSERT(node);
  const std::string name = prefix + node->name();
  bool includeParent = (node->begin() == node->end()) || !leavesOnly;
  ProfileData data(node->data());
  std::string name2add = name;
  if (!includeParent && leavesOnly && (node->name() != "root")) {
      // check whether leaf nodes represent more than 99% of parent's execution time
      // add the remainder explicitly if it is not the case. root is always added as
      // it is handy to have the overall timing stats
      // min/max stats for the remainder will not be very useful
      double totalTimeLeafNodes = 0.;
      for (ProfileNode::iterator it = node->begin(); it != node->end(); ++it) {
           totalTimeLeafNodes += it->data().totalTime();
      }
      if (totalTimeLeafNodes < node->data().totalTime()*0.99) {
          includeParent = true;
          name2add += ".remainder";
          ProfileData remainder(data.totalTime() - totalTimeLeafNodes);
          remainder.setCount(data.count());
          data = remainder;
      } 
  }
  if (includeParent || (node->name() == "root")) {
      std::map<std::string, ProfileData>::iterator it = stats.find(name2add);
      if (it == stats.end()) {
          // new element in the map
          stats[name2add] = data;      
      } else {
          ASKAPCHECK(!doHierarchy, "Duplicated key in the statistics map, this shouldn't happen in the hierarchy mode!");
          // merge in the current node
          it->second.add(data);
      }
  }
  for (ProfileNode::iterator it = node->begin(); it != node->end(); ++it) {
       boost::shared_ptr<ProfileNode> thisNode(&(*it), utility::NullDeleter());
       extractStats(stats, doHierarchy ? name + "." : "", thisNode, doHierarchy, leavesOnly); 
  }
}

/// @brief copy constructor
/// @details We do not allow to copy profile tree which has accumulated some data.
/// This is done to avoid referencing issues when shared pointers are copied.
/// However, we need a copy constructor to be able to keep thread trees in the map.
/// @param[in] other another instance of the tree
ProfileTree::ProfileTree(const ProfileTree &other) : itsRootNode(other.itsRootNode),
    itsCurrentNode(&itsRootNode, utility::NullDeleter()) {}

/// @brief assignment operator
/// @details throws an exception if called
/// @param[in] other another instance of the tree
ProfileTree& ProfileTree::operator=(const ProfileTree &other) 
{
  ASKAPTHROW(AskapError, "ProfileTree::operator= is not implemented");
  return *this;
}


