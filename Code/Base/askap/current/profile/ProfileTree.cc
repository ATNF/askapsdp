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

