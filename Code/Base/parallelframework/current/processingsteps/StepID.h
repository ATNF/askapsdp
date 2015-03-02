/// @file 
/// @brief Range of ranks identifier to be assigned for parallel steps
/// @details The framework sets up relations between different parallel steps
/// through communicators. The rank allocation has some flexibility and is not
/// known until initialise method of the composite step. Moreover, more than
/// one rank can be allocated to a single processing step. This class helps to
/// keep track the range of ranks and acts as an ID to identify a particular step.
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

#ifndef ASKAP_ASKAPPARALLEL_STEP_ID_H
#define ASKAP_ASKAPPARALLEL_STEP_ID_H


namespace askap {

namespace askapparallel {
	
/// @brief Range of ranks identifier to be assigned for parallel steps
/// @details The framework sets up relations between different parallel steps
/// through communicators. The rank allocation has some flexibility and is not
/// known until initialise method of the composite step. Moreover, more than
/// one rank can be allocated to a single processing step. This class helps to
/// keep track the range of ranks and acts as an ID to identify a particular step.
class StepID 
{
public:
	
   /// @brief empty constructor, creates single rank step at rank zero
   /// @details We need the empty constructor to be able to store this type
   /// in containers
   StepID();

   /// @brief construct an object assigned to a singe rank
   /// @details 
   /// @param[in] rank rank to assign
   explicit StepID(int rank);

   /// @brief construct an object with explicit first and last ranks
   /// @details
   /// @param[in] first first rank
   /// @param[in] last last rank
   /// @param[in] nRanks number of ranks allocated as a group
   StepID(int first, int last, unsigned int nRanks = 1u);

   /// @brief first rank
   /// @return first rank allocated to this step
   inline int first() const { return itsFirst;}

   /// @brief last rank
   /// @return last rank allocated to this step
   inline int last() const { return itsLast;}

   /// @brief number of ranks in the group
   /// @return number of ranks allocated as a group to the processing step
   inline unsigned int nRanks() const { return itsNRanks;}

   /// @brief check whether this is a single rank step
   /// @return true, if the step represented by this object is
   /// allocated a single rank.
   inline bool isSingleRank() const 
       { return (itsFirst == itsLast) && (itsNRanks == 1u); }

   /// @brief extract single rank from this allocation
   /// @details The whole allocation can be represented as a 
   /// number of groups each containing itsNRanks elements.
   /// This operator returns a single rank StepID corresponding
   /// to the given group and element.
   /// @param[in] group zero-based group number to choose
   /// @param[in] element zero-based element number to choose
   /// @note For a flexible allocation it is impossible to check
   /// the validity of the result inside this method. It is checked
   /// when the logical allocation is translated to the physical 
   /// allocation. However, the element should always be between
   /// zero and itsNRanks-1, inclusive, and this is checked.
   StepID operator()(unsigned int group, unsigned int element = 0) const;

   /// @brief project a flexible allocation to a given number of ranks
   /// @details The total number of ranks is not known when logical 
   /// relations are set up. This might result in a flexible allocation
   /// of ranks for a given processing step or allocation w.r.t. the end
   /// of rank space (i.e. the last available rank). This method maps the
   /// allocation represented by this object to the given number of ranks
   /// available. For fixed allocations checks are done that ranks do not
   /// exceed the total number available.
   /// @param[in] nRanks total number of ranks available
   void project(unsigned int nRanks);

private:

   /// @brief helper method to project a single index
   /// @details Used in the project method.
   /// @param[in] index index to project
   /// @param[in] nRanks total number of ranks available
   /// @return rank corresponding to the given index
   static int projectIndex(int index, int nRanks);

  
   /// @brief first rank assigned to this step
   /// @note negative values point to ranks counted from the very last available. 
   /// We cannot map all processing steps to available rank space until initialise
   /// call. To allow flexible allocation (of all available ranks or multiple copies
   /// of the step), we use negative values as necessary to create links between 
   /// processes and put actual ranks in the initialise calls.
   int itsFirst;

   /// @brief last rank assigned to this step
   /// @note negative values point to ranks counted from the very last available. 
   /// We cannot map all processing steps to available rank space until initialise
   /// call. To allow flexible allocation (of all available ranks or multiple copies
   /// of the step), we use negative values as necessary to create links between 
   /// processes and put actual ranks in the initialise calls.
   int itsLast;

   /// @brief number of ranks logically assigned to the same processing step
   /// @details
   /// itsFirst and itsLast specify the full range of ranks allocated to a
   /// processing step. There could be both multiple instances and multirank
   /// processing steps. From the point of rank allocation, these cases are
   /// equivalent (as each rank has a separate copy of the processing step
   /// object anyway). But we must be able to distinguish between these cases
   /// when we set up connections. itsNRanks speficies the number of ranks allocated
   /// as a group to processing step.
   unsigned int itsNRanks;
};

} // end of namespace askapparallel
} // end of namespace askap
#endif // #ifndef ASKAP_ASKAPPARALLEL_STEP_ID_H

