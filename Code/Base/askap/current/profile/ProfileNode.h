/// @file
/// @brief node of the tree with profile information
///
/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This class represent a single node of the tree corresponding to one method call. 
/// Every calls to traceable methods within the given method are dealt with by the child nodes.
/// Each node has a name, profile data, a map of optional lower level nodes and
/// a debug level (an integer to be able to filter out the required info easily). Each node
/// also has a shared pointer to the higher level node (so the tree cursor can navigate around).
/// An uninitialised shared pointer correspond to the top level node.
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

#ifndef ASKAP_PROFILE_NODE_H
#define ASKAP_PROFILE_NODE_H

// std includes
#include <string>

// own includes
#include <askap/AskapUtil.h>
#include <profile/ProfileData.h>

// boost includes
#include <boost/shared_ptr.hpp>


namespace askap {

/// @brief node of the tree with profile information
///
/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This class represent a single node of the tree corresponding to one method call. 
/// Every calls to traceable methods within the given method are dealt with by the child nodes.
/// Each node has a name, profile data, a map of optional lower level nodes and
/// a debug level (an integer to be able to filter out the required info easily). Each node
/// also has a shared pointer to the higher level node (so the tree cursor can navigate around).
/// An uninitialised shared pointer correspond to the top level node.
/// @ingroup profile
class ProfileNode {
public:
   /// @brief default constructor for an empty node
   ProfileNode();
   
   /// @brief constructor of a node with the given name and an optional parent
   /// @details
   /// @param[in] name name of the method corresponding to this node
   /// @param[in] parent shared pointer to the parent node (default is no parent)
   explicit ProfileNode(const std::string &name, const boost::shared_ptr<ProfileNode> & parent = 
                        boost::shared_ptr<ProfileNode>());
   
   /// @brief access to data 
   /// @return reference to the data
   inline ProfileData& data() { return itsData;}
   
   /// @return name of this node
   inline const std::string& name() const { return itsName;}
   
   /// @return shared pointer to the parent
   /// @note uninitialised shared pointer is a signature of the root node
   inline const boost::shared_ptr<ProfileNode>& parent() const { return itsParent;}
   
   /// @brief child node with the given name
   /// @details This method returns the child node with the given name. If no
   /// such child exists, an empty node is created and returned.
   /// @param[in] name name of the child node
   /// @return shared pointer to the child node
   boost::shared_ptr<ProfileNode> child(const std::string &name);
   
   
private:   
   /// @brief profiling statistics
   ProfileData itsData;
   
   /// @brief name of this node
   std::string itsName;
   
   /// @brief parent of the current node
   /// @note uninitialised shared pointer is a signature of the root node
   boost::shared_ptr<ProfileNode> itsParent;
   
   /// @brief list of child nodes in the tree
   /// @details Each element correspond to a method call from within the method
   /// represented by this node. 
   std::list<ProfileNode> itsChildren;   
};

} // namespace askap

#endif // #ifndef ASKAP_PROFILE_NODE_H
