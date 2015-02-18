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

// own includes
#include "processingsteps/StepID.h"
#include <askap/AskapError.h>

namespace askap {

namespace askapparallel {
	
/// @brief empty constructor, creates single rank step at rank zero
/// @details We need the empty constructor to be able to store this type
/// in containers
StepID::StepID() : itsFirst(0), itsLast(0), itsNRanks(1u) {}

/// @brief construct an object assigned to a singe rank
/// @details 
/// @param[in] rank rank to assign
StepID::StepID(int rank) : itsFirst(rank), itsLast(rank), itsNRanks(1u) {}

/// @brief construct an object with explicit first and last ranks
/// @details
/// @param[in] first first rank
/// @param[in] last last rank
/// @param[in] nRanks number of ranks allocated as a group
StepID::StepID(int first, int last, unsigned int nRanks) : itsFirst(first), 
   itsLast(last), itsNRanks(nRanks)
{
  ASKAPCHECK(itsNRanks > 0, "Number of ranks in a group should be positive");
  if ((itsFirst < 0) == (itsLast < 0)) {
     // this is a pre-defined rank allocation, not a flexible one where we don't
     // know exact number of ranks until initialise call
     ASKAPCHECK(itsLast >= itsFirst, "Expect the last rank to be grater than or equal than the first rank, you have first="<<
          itsFirst<<" and last="<<itsLast);
     const unsigned int allRanks = itsLast - itsFirst + 1;
     ASKAPCHECK(allRanks >= itsNRanks, "Cannot allocate "<<itsNRanks<<" rank groups for only "<<allRanks<<" ranks"); 
     ASKAPCHECK(allRanks % itsNRanks == 0, "Fixed uneven allocation of multirank processing steps, logical error is suspected");
  }
  if (itsFirst < 0) {
     // this is a flexible allocation, the end should also be allocated w.r.t. the end of rank space
     ASKAPCHECK(itsLast < 0, "First rank is defined w.r.t. the end of rank space, first="<<itsFirst<<
            ", however the last rank is given explicitly, last="<<itsLast<<". This shouldn't happen");
  }
}


} // end of namespace askapparallel
} // end of namespace askap

