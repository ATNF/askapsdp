/// @file 
///
/// @brief This is a template providing a basic X-step of a correlator
/// @details For BETA-3 experiments we want to be able to correlate the data
/// in software. This templated class implements a core functionality of a
/// single baseline correlator computing just the correlation matrix. The
/// interface is quite generic, so we can use it for both on-the-fly and 
/// off-line correlation. In addition, the dependency on other libraries has
/// been minimised (so we can integrate the class with the recording stage, if
/// we decide to do so later on).
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

#ifndef SIMPLE_CORRELATOR_TCC
#define SIMPLE_CORRELATOR_TCC

namespace askap {

/// @brief reset accumulator, adjust delays if necessary
/// @details This method is equivalent to the constructor, but it doesn't change the number
/// of delay steps supported by the class
/// @param[in] delay1 delay (in samples) for the first stream
/// @param[in] delay2 delay (in samples) for the second stream
/// @note the buffers are treated as parts of the continuous
/// stream. Incomplete buffers are ignored for simplicity.
template<typename AccType, typename IndexType>         
void SimpleCorrelator<AccType, IndexType>::reset(const IndexType delay1, const IndexType delay2) 
{
  this->itsDelay = delay2 - delay1;
  this->reset();
}

/// @brief just reset accumulator
/// @details This method can be used to move to the next integration cycle
template<typename AccType, typename IndexType>         
void SimpleCorrelator<AccType, IndexType>::reset() 
{
  for (typename std::vector<AccType>::iterator it = this->itsAccumulator.begin(); it != this->itsAccumulator.end(); ++it) {
       *it = AccType(0);
  }
}

/// @brief accumulate buffers
/// @details 
/// The parameter of the template is as follows.
///    Iter - type of the read-only random-access iterator to read the data buffer
///           (it could be just an ordinary pointer)
/// @param[in] stream1 start iterator of the first stream
/// @param[in] stream2 start iterator of the second stream
/// @param[in] size number of samples
template<typename AccType, typename IndexType>         
template<typename Iter>
void SimpleCorrelator<AccType, IndexType>::accumulate(const Iter stream1, const Iter stream2, const IndexType size)
{
  IndexType offset1 = itsDelay < 0 ? -itsDelay : 0;
  IndexType offset2 = itsDelay > 0 ? itsDelay : 0;
  if (itsNDelays == 1) {
    // special case of a single delay = 0 step (i.e. just cross-correlation of two streams)
    Iter offsetStream1 = stream1 + offset1;
    Iter offsetStream2 = stream2 + offset2;
    AccType result = 0;
    for (; (offset1 < size) && (offset2 < size); ++offset1, ++offset2, ++offsetStream1, ++offsetStream2) {
         result += AccType(*offsetStream1) * conj(AccType(*offsetStream2));
    }
    itsAccumulator[0] = result;
  } else {
    // general case of multiple delay steps
    for (; (offset1 + itsNDelays < size) && (offset2 + itsNDelays < size); offset1 += itsNDelays, offset2 += itsNDelays) {
       typename std::vector<AccType>::iterator it = itsAccumulator.begin();
       Iter offsetStream1 = stream1 + offset1;
       Iter offsetStream2 = stream2 + offset2;
       for (IndexType i = 0; i<itsNDelays; ++i, ++offsetStream1) {
            const AccType first = *offsetStream1;
            for (IndexType j = 0; j<=i; ++j, ++it) {
                 const AccType second = *(offsetStream2 + j);
                 *it += first * std::conj(second);
            }
       }
    }
  }  
}


/// @brief constructor, optionally setup initial delays
/// @details 
/// @param[in] delay1 delay (in samples) for the first stream
/// @param[in] delay2 delay (in samples) for the second stream
/// @param[in] delay3 delay (in samples) for the third stream
/// @note the buffers are treated as parts of the continuous
/// stream. Incomplete buffers are ignored for simplicity.
template<typename AccType, typename IndexType>         
Simple3BaselineCorrelator<AccType, IndexType>::Simple3BaselineCorrelator(const IndexType delay1, 
         const IndexType delay2, const IndexType delay3) : itsDelay1(delay1), itsDelay2(delay2),
         itsDelay3(delay3), itsVis12(AccType(0)), itsVis13(AccType(0)), itsVis23(AccType(0)) 
{
  const IndexType minDelay = min(itsDelay1, min(itsDelay2, itsDelay3));
  itsDelay1 -= minDelay;
  itsDelay2 -= minDelay;
  itsDelay3 -= minDelay;  
}

/// @brief reset accumulator, adjust delays
/// @details This method is equivalent to the constructor
/// @param[in] delay1 delay (in samples) for the first stream
/// @param[in] delay2 delay (in samples) for the second stream
/// @param[in] delay3 delay (in samples) for the third stream
/// @note the buffers are treated as parts of the continuous
/// stream. Incomplete buffers are ignored for simplicity.
template<typename AccType, typename IndexType>         
void Simple3BaselineCorrelator<AccType, IndexType>::reset(const IndexType delay1, const IndexType delay2, const IndexType delay3)
{
  const IndexType minDelay = min(itsDelay1, min(itsDelay2, itsDelay3));
  itsDelay1 -= minDelay;
  itsDelay2 -= minDelay;
  itsDelay3 -= minDelay;  
  reset();
}

/// @brief just reset accumulator
/// @details This method can be used to move to the next integration cycle
template<typename AccType, typename IndexType>         
void Simple3BaselineCorrelator<AccType, IndexType>::reset()
{
  itsVis12 = AccType(0);
  itsVis13 = AccType(0);
  itsVis23 = AccType(0);  
}


/// @brief accumulate buffers
/// @details 
/// The parameter of the template is as follows.
///    Iter - type of the read-only random-access iterator to read the data buffer
///           (it could be just an ordinary pointer)
/// @param[in] stream1 start iterator of the first stream
/// @param[in] stream2 start iterator of the second stream
/// @param[in] stream3 start iterator of the second stream
/// @param[in] size number of samples
template<typename AccType, typename IndexType>         
template<typename Iter>
void Simple3BaselineCorrelator<AccType, IndexType>::accumulate(const Iter stream1, const Iter stream2, 
            const Iter stream3, const IndexType size)
{
  IndexType offset1 = itsDelay1;
  IndexType offset2 = itsDelay2;
  IndexType offset3 = itsDelay3;  

  for (; (offset1 < size) && (offset2 < size) && (offset3 < size); ++offset1, ++offset2, ++offset3) {
       itsVis12 += AccType(*(stream1 + offset1)) * conj(AccType(*(stream2+offset2)));
       itsVis13 += AccType(*(stream1 + offset1)) * conj(AccType(*(stream3+offset3)));
       itsVis23 += AccType(*(stream2 + offset2)) * conj(AccType(*(stream3+offset3)));
  }
}            


} // namespace askap

#endif // #ifndef SIMPLE_CORRELATOR_TCC


