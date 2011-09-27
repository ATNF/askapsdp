/// @file
/// @brief iterator adapter allowing breaks after given time interval
///
/// @details This adapter is derived from DataIteratorAdapter. The logic
/// of hasMore method is modified that the adapter signals the end of the
/// iteration when a certain time interval since the previous stop is 
/// reached. The iteration can be resumed afterwards, provided there are
/// more data still available. The assumption is that the data are 
/// time-ordered. An exception is thrown if it is not the case.
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

#include <dataaccess/TimeChunkIteratorAdapter.h>

namespace askap {

namespace accessors {

/// @brief default constructor to get an uninitialised adapter
TimeChunkIteratorAdapter::TimeChunkIteratorAdapter() : itsCurrentChunkTime(0.), itsPrevTime(0.),
            itsInterval(-1.), itsChangeMonitor(changeMonitor()) {} 
  
/// @brief setup with the given iterator
/// @details
/// @param[in] iter shared pointer to iterator to be wrapped
/// @param[in] interval maximum time separation of individual chunks (in seconds)
/// @note the code tries to cast the shared pointer to a non-const iterator type. If
/// successul, non-const methods of the adapter will also work. Negative interval
/// means infinite time interval (making this adapter equivalent to the plain
/// DataIteratorAdapter)
TimeChunkIteratorAdapter::TimeChunkIteratorAdapter(const boost::shared_ptr<IConstDataIterator> &iter, 
           const double interval) : DataIteratorAdapter(iter), itsCurrentChunkTime(0.), itsPrevTime(0.),
           itsInterval(interval), itsChangeMonitor(changeMonitor()) 
{
  ASKAPCHECK(iter, "An attempt to initialise TimeChunkIteratorAdapter with empty shared pointer");
  itsCurrentChunkTime = (*iter)->time();
  itsPrevTime = itsCurrentChunkTime;
}           

/// @brief set new maximum chunk time span 
/// @details 
/// @param[in] interval maximum time separation of individual chunks (in seconds)
/// @note Negative interval means infinite time interval (making this adapter 
/// equivalent to the plain DataIteratorAdapter). It is assumed that this method is
/// used either before the actual use of the adapter or when iteration is broken after
/// hasMore returned true.
void TimeChunkIteratorAdapter::setInterval(const double interval)
{
  itsInterval = interval;
}

  
/// @brief Checks whether there are more data available in this chunk
/// @return True if there are more data available in this chunk
/// @note for this particular adapter this method corresponds to the
/// current chunk rather than to all dataset
casa::Bool TimeChunkIteratorAdapter::hasMore() const throw()
{
  if (itsChangeMonitor != changeMonitor() || (itsInterval < 0)) {
      return DataIteratorAdapter::hasMore();
  }
  const double curTime = roIterator()->time();
  ASKAPDEBUGASSERT((curTime >= itsCurrentChunkTime));
  return DataIteratorAdapter::hasMore() && (curTime - itsCurrentChunkTime < itsInterval);
}
  
/// advance the iterator one step further 
/// @return True if there are more data (so constructions like 
///         while(it.next()) {} are possible)
casa::Bool TimeChunkIteratorAdapter::next()
{
  ASKAPCHECK(hasMore(), "There are no more data available in this chunk (or at all, if resume method has been called)");
  const double curTime = roIterator()->time();
  if (itsChangeMonitor != changeMonitor()) {
      // the iterator has been updated, need to start new chunk
      itsChangeMonitor = changeMonitor();
      itsCurrentChunkTime = curTime;
      itsPrevTime = curTime;
  }
  ASKAPCHECK(curTime >= itsPrevTime, 
      "Data appear to be not in time order, TimeChunkIteratorAdapter can't handle this situation. Last time = "<<
      itsPrevTime<<" s, current time = "<<curTime);
  itsPrevTime = curTime;  
  return DataIteratorAdapter::next();
}
  
/// @brief checks whether there are more data available
/// @details This method disregards the split into time chunks.
/// @return true if there are more data available 
bool TimeChunkIteratorAdapter::moreDataAvailable() const
{
  return DataIteratorAdapter::hasMore();
}
  
/// @brief resume iteration (proceed to next chunk)
/// @details A call to this method resets hasMore flag, so iteration
/// can continue until the end of the following chunk (or the end of the data)
void TimeChunkIteratorAdapter::resume() const
{
   ASKAPCHECK(moreDataAvailable(), "Unable to resume iteration as no more data are available");
   itsCurrentChunkTime = roIterator()->time();
   itsPrevTime = itsCurrentChunkTime;  
}


} // namespace accessors

} // namespace askap

