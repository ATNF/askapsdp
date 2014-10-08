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


// own include
#include <utils/MultiDimPosIter.h>

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

using namespace askap;
using namespace askap::scimath;

/// @brief empty iterator 
MultiDimPosIter::MultiDimPosIter() : itsHasMore(false) {}
  
/// @brief construct iterator and set the cursor at origin
/// @details
/// @param[in] shape shape of the volume to iterate
MultiDimPosIter::MultiDimPosIter(const casa::IPosition &shape) : itsHasMore(!shape.empty()),
    itsStart(shape.nelements(),0), itsEnd(shape), itsCursor(shape.nelements(),0), itsShape(shape)
{
   for (casa::uInt dim = 0; dim < itsEnd.nelements(); ++dim) {
        --itsEnd[dim];
        ASKAPDEBUGASSERT(itsStart[dim] <= itsEnd[dim]);
   }
}     
  
/// @brief construct iterator with the given start and stop positions
/// @details
/// @param[in] shape shape of the volume to iterate
/// @param[in] start position of the origin
/// @param[in] end end position
MultiDimPosIter::MultiDimPosIter(const casa::IPosition &shape, const casa::IPosition &start, const casa::IPosition &end) :
  itsHasMore(!start.empty()), itsStart(start), itsEnd(end), itsCursor(start), itsShape(shape) 
{
   ASKAPCHECK(start.nelements() == end.nelements(), 
       "Start and end points of the MultiDimPosIter should have the same dimensionality");
   ASKAPCHECK(start.nelements() == shape.nelements(),
       "Shape, start and end are supposed to have the same dimensionality");
   casa::Int64 flatStart = 0, flatEnd = 0;         
   for (casa::uInt dim = 0; dim < start.nelements(); ++dim) {
        ASKAPCHECK(start[dim] <= shape[dim], "Start point exceeds the shape: "<<start<<" "<<shape);
        ASKAPCHECK(end[dim] <= shape[dim], "End point exceeds the shape: "<<end<<" "<<shape);
        const casa::Int64 factor = (dim > 0 ? shape.getFirst(dim).product() : 1);
        flatStart += start[dim] * factor;
        flatEnd += end[dim] * factor;
   } 
   ASKAPCHECK(flatStart <= flatEnd, "Start seems to be earlier than end");   
}

/// @brief initialise to iterate over full volume
/// @details
/// @param[in] shape shape of the volume to iterate
void MultiDimPosIter::init(const casa::IPosition &shape)
{
  itsHasMore = !shape.empty();
  itsStart = casa::IPosition(shape.nelements(),0);
  itsEnd = shape;
  itsShape = shape;
  itsCursor = itsStart;
  for (casa::uInt dim = 0; dim < itsEnd.nelements(); ++dim) {
       --itsEnd[dim];
       ASKAPDEBUGASSERT(itsStart[dim] <= itsEnd[dim]);
  }  
}
  
/// @brief initialise to iterate over given range
/// @details
/// @param[in] shape shape of the volume to iterate
/// @param[in] start position of the origin
/// @param[in] end end position
void MultiDimPosIter::init(const casa::IPosition &shape, const casa::IPosition &start, const casa::IPosition &end)
{
  ASKAPCHECK(start.nelements() == end.nelements(), 
       "Start and end points of the MultiDimPosIter should have the same dimensionality");
  ASKAPCHECK(start.nelements() == shape.nelements(),
       "Shape, start and end are supposed to have the same dimensionality");     
  casa::Int64 flatStart = 0, flatEnd = 0;         
  for (casa::uInt dim = 0; dim < start.nelements(); ++dim) {
       ASKAPCHECK(end[dim] <= shape[dim], "End point exceeds the shape: "<<end<<" "<<shape);
       const casa::Int64 factor = (dim > 0 ? shape.getFirst(dim).product() : 1);
       flatStart += start[dim] * factor;
       flatEnd += end[dim] * factor;
  }    
  ASKAPCHECK(flatStart <= flatEnd, "Start seems to be earlier than end");   
  itsHasMore = !start.empty();
  itsStart = start;
  itsEnd = end;
  itsCursor = start;  
  itsShape = shape; 
}

/// @brief advance iterator to the next point
/// @note An exception is thrown if no more points are available
void MultiDimPosIter::next()
{
  ASKAPCHECK(itsHasMore, "MultiDimPosIter doesn't have more points in the iteration range");
  if (itsCursor == itsEnd) {
      // the iteration is over
      itsHasMore = false;
      return;
  }
  for (casa::uInt dim = 0; dim < itsShape.nelements(); ++dim) {
       ++itsCursor[dim];
       if (itsCursor[dim] < itsShape[dim]) {
           return;
       }
       itsCursor[dim] = 0;   
  }
  itsHasMore = false;
  // actually we're not supposed to reach this point.
  ASKAPTHROW(AskapError, "Logic error - the code is not supposed to reach this point");
}
  
/// @brief rewind the iterator to the origin
void MultiDimPosIter::origin()
{
  if (itsStart.empty()) {
      itsHasMore = false;
  } else {
      itsCursor = itsStart;
      itsHasMore = true;
  }
}

/// @brief initialise iterator to the empty range
void MultiDimPosIter::init()
{
  itsHasMore = false;
  itsStart = casa::IPosition();
  itsCursor = itsStart;
  itsEnd = itsStart;
  itsShape = itsEnd;
}

/// @brief initialise to iterate over one chunk of the full range
/// @details This method bins the whole iteration range into a given number of
/// chunks and sets up iteration over selected chunk.
/// @note The resulting range may be empty.
/// @param[in] shape shape of the volume to iterate
/// @param[in] nChunks desired total number of chunks
/// @param[in] chunk selected chunk number (0..nChunk-1)
void MultiDimPosIter::init(const casa::IPosition &shape, const casa::uInt nChunks, const casa::uInt chunk)
{
  ASKAPCHECK(chunk < nChunks, "Selected chunk = "<<chunk<<" is outside the range; nChunks="<<nChunks);
  ASKAPCHECK(nChunks > 0, "Number of chunks is supposed to be positive");
  if (shape.empty()) {
      init();
  } else {
      const casa::Int64 totalNumber = shape.product();
      ASKAPDEBUGASSERT(totalNumber > 0);
      const casa::uInt pointsPerChunk = static_cast<casa::uInt>(totalNumber / nChunks) + (totalNumber % nChunks == 0 ? 0 : 1);
      casa::Int64 flatStart = pointsPerChunk * chunk;
      if (flatStart >= totalNumber) {
          // in this case just revert to empty iteration - we have an unbalanced case with more chunks than we need
          init();
      } else {
          casa::Int64 flatStop = pointsPerChunk * (chunk + 1) - 1;
      
          if (flatStop >= totalNumber) {
              flatStop = totalNumber - 1;
          }
          casa::IPosition start(shape.nelements(),0);
          casa::IPosition end = start; 
      
          for (casa::uInt dim = start.nelements() - 1; dim > 0; --dim) {
               const casa::Int64 factor = shape.getFirst(dim).product(); 
               start[dim] = flatStart / factor;
               flatStart -= start[dim] * factor;
               end[dim] = flatStop / factor;
               flatStop -= end[dim] * factor;           
          }
          start[0] = flatStart;
          end[0] = flatStop;
          init(shape, start,end);
      }     
  }
}


