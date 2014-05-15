/// @file
/// 
/// @brief helper iterator class to traversing multi-dimensional indices
/// @details The idea behind this class is similar to that of casa::ArrayPositionIterator, but 
/// with some support of incomplete iterations over some dimension. Moreover, there is 
/// functionality to partition complete iteration into given number of groups. This is handy
/// to distribute work defined by more than one number between workers. 
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

#ifndef ASKAP_SCIMATH_MULTI_DIM_POS_ITER_H
#define ASKAP_SCIMATH_MULTI_DIM_POS_ITER_H

#include <casa/Arrays/IPosition.h>

namespace askap {

namespace scimath {

/// @brief helper iterator class to traversing multi-dimensional indices
/// @details The idea behind this class is similar to that of casa::ArrayPositionIterator, but 
/// with some support of incomplete iterations over some dimension. Moreover, there is 
/// functionality to partition complete iteration into given number of groups. This is handy
/// to distribute work defined by more than one number between workers. 
/// @ingroup utils
struct MultiDimPosIter {

  /// @brief empty iterator 
  MultiDimPosIter();
  
  /// @brief construct iterator and set the cursor at origin
  /// @details
  /// @param[in] shape shape of the volume to iterate
  explicit MultiDimPosIter(const casa::IPosition &shape);
  
  /// @brief construct iterator with the given start and stop positions
  /// @details
  /// @param[in] shape shape of the volume to iterate
  /// @param[in] start position of the origin
  /// @param[in] end end position
  MultiDimPosIter(const casa::IPosition &shape, const casa::IPosition &start, const casa::IPosition &end);
  
  /// @brief initialise to iterate over full volume
  /// @details
  /// @param[in] shape shape of the volume to iterate
  void init(const casa::IPosition &shape);
  
  /// @brief initialise to iterate over given range
  /// @details
  /// @param[in] shape shape of the volume to iterate
  /// @param[in] start position of the origin
  /// @param[in] end end position
  void init(const casa::IPosition &shape, const casa::IPosition &start, const casa::IPosition &end);
  
  /// @brief initialise to iterate over one chunk of the full range
  /// @details This method bins the whole iteration range into a given number of
  /// chunks and sets up iteration over selected chunk.
  /// @note The resulting range may be empty.
  /// @param[in] shape shape of the volume to iterate
  /// @param[in] nChunks desired total number of chunks
  /// @param[in] chunk selected chunk number (0..nChunk-1)
  void init(const casa::IPosition &shape, const casa::uInt nChunks, const casa::uInt chunk);
  
  /// @brief obtain current cursor
  /// @return const reference to IPosition object
  inline const casa::IPosition& cursor() const { return itsCursor; }
  
  /// @brief check whether the iterator has more points to iterate
  /// @return true, if there are more points to iterate
  inline bool hasMore() const { return itsHasMore; }
  
  /// @brief advance iterator to the next point
  /// @note An exception is thrown if no more points are available
  void next();
  
  /// @brief rewind the iterator to the origin
  void origin();

private:
   
  /// @brief flag indicating that more data are available
  bool itsHasMore;
  
  /// @brief start of the range
  casa::IPosition itsStart;
  
  /// @brief last point of the range
  casa::IPosition itsEnd;
  
  /// @brief current cursor
  casa::IPosition itsCursor; 
  
  /// @brief shape of the iteration range
  casa::IPosition itsShape;

}; // MultiDimPosIter

} // namespace scimath

} // namespace askap

#endif // #ifndef ASKAP_SCIMATH_MULTI_DIM_POS_ITER_H

