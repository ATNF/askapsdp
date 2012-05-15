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

#ifndef ASKAP_PROFILE_DATA_H
#define ASKAP_PROFILE_DATA_H

namespace askap {

/// @details The profile package is used to accumulate statistics
/// (e.g. to produce a pie chart to investigate where processing time goes)
/// This class represent a data structure which is accumulated for every
/// selected method.
/// @ingroup profile
struct ProfileData {
   /// @brief default constructor
   /// @details Initialises the node with default values of zero execution time and
   /// zero number of calls
   ProfileData();
   
   /// @brief constructor corresponding to the first call
   /// @details This version initialises the number of calls to one and the times to
   /// the given execution times of the first call
   /// @param[in] time execution time for the first call
   explicit ProfileData(const double time);

   // access to the stats
   /// @return number of calls
   inline long count() const { return itsCount; }

   /// @return total execution time
   inline double totalTime() const { return itsTotalTime; }

   /// @return longest execution time
   inline double maxTime() const { return itsMaxTime; }

   /// @return shortest execution time
   inline double minTime() const { return itsMinTime; }
   
   /// @brief process another execution
   /// @details This method increments total time and count and
   /// adjusts min/max statistics as required. It is called after each run of the 
   /// traced method.
   /// @param[in] time execution time for this call
   void add(const double time);

   /// @brief process another execution
   /// @details This method merges in an additional object of the same type
   /// @param[in] other another ProfileData object
   void add(const ProfileData &other);
   
private:
   /// @brief number of calls
   long itsCount;
   /// @brief total execution time
   double itsTotalTime;
   /// @brief longest execution time
   /// @note it is undefined if itsCount == 0
   double itsMaxTime;
   /// @brief shortest execution time
   /// @note it is undefined if itsCount == 0
   double itsMinTime;
};

} // namespace askap


#endif // #ifndef ASKAP_PROFILE_DATA_H


