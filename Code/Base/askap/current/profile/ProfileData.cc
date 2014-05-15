/// @file
/// @brief data structure accumulated as part of the profile
///
/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This class represent a data structure which is accumulated for every
/// selected method.
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

// own includes
#include <profile/ProfileData.h>

using namespace askap;

/// @brief default constructor
/// @details Initialises the node with default values of zero execution time and
/// zero number of calls
ProfileData::ProfileData() : itsCount(0), itsTotalTime(0.), itsMaxTime(0.), itsMinTime(0.) {}

/// @brief constructor corresponding to the first call
/// @details This version initialises the number of calls to one and the times to
/// the given execution times of the first call
/// @param[in] time execution time for the first call
ProfileData::ProfileData(const double time) : itsCount(1), itsTotalTime(time), itsMaxTime(time), 
                itsMinTime(time) {}

                
/// @brief process another execution
/// @details This method increments total time and count and
/// adjusts min/max statistics as required. It is called after each run of the 
/// traced method.
/// @param[in] time execution time for this call
void ProfileData::add(const double time) 
{
   if (itsCount > 0) {
       if (time > itsMaxTime) {
           itsMaxTime = time;
       }
       if (time < itsMinTime) {
           itsMinTime = time;
       }
   } else {
       itsMaxTime = time;
       itsMinTime = time;
   }
   itsTotalTime += time;
   ++itsCount;
}

/// @brief process another execution
/// @details This method merges in an additional object of the same type
/// @param[in] other another ProfileData object
void ProfileData::add(const ProfileData &other) {
   if (other.itsCount > 0) {
       if (itsCount > 0) {
           if (itsMaxTime < other.itsMaxTime) {
               itsMaxTime = other.itsMaxTime;
           }
           if (itsMinTime > other.itsMinTime) {
               itsMinTime = other.itsMinTime;
           }
       } else {
           itsMaxTime = other.itsMaxTime;
           itsMinTime = other.itsMinTime;
       }
       itsTotalTime += other.itsTotalTime;
       itsCount += other.itsCount;
   }
}

                
