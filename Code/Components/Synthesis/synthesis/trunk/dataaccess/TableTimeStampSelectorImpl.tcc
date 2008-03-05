/// @file 
/// @brief Implementation of the time range selector
/// @details This file contain implementation and explicit specializations
/// of the TableTimeStampSelectorImpl template
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <casa/Quanta/MVEpoch.h>
#include <dataaccess/IDataConverterImpl.h>

namespace askap {

namespace synthesis {

/// @brief construct time range selector
/// @param[in] start start time of the interval
/// @param[in] stop stop time of the interval
template<typename T>
TableTimeStampSelectorImpl<T>::TableTimeStampSelectorImpl(const casa::Table &tab, 
       const T &start, const T &stop) : TableHolder(tab), itsStart(start), 
                                        itsStop(stop) {}

// general implementation, works  for both MVEpoch and Double
template<typename T>
std::pair<casa::MEpoch, casa::MEpoch>
           TableTimeStampSelectorImpl<T>::getStartAndStop() const       
{
  const IDataConverterImpl &conv = converter();
  return std::pair<casa::MEpoch,casa::MEpoch>(conv.epochMeasure(itsStart),
                        conv.epochMeasure(itsStop));
}


} // namespace synthesis

} // namespace askap
