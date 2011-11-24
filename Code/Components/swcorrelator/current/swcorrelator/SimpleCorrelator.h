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

#ifndef SIMPLE_CORRELATOR_H
#define SIMPLE_CORRELATOR_H

#include <iterator>
#include <stdexcept>
#include <vector>
#include <complex>

namespace askap {

namespace swcorrelator {

/// @brief This is a template providing a basic X-step of a correlator
/// @details For BETA-3 experiments we want to be able to correlate the data
/// in software. This templated class implements a core functionality of a
/// single baseline correlator computing just the correlation matrix. The
/// interface is quite generic, so we can use it for both on-the-fly and 
/// off-line correlation. In addition, the dependency on other libraries has
/// been minimised (so we can integrate the class with the recording stage, if
/// we decide to do so later on).
///
///    AccType - type of the accumulated values (may be different from the input data type 
///              to allow overflow)
///    IndexType - type of the sample index 
/// @ingroup swcorrelator
template<typename AccType = std::complex<float>, typename IndexType = int>         
class SimpleCorrelator {
public:
  /// @brief constructor, optionally setup initial delays
  /// @details 
  /// @param[in] nDelays number of delay steps
  /// @param[in] delay1 delay (in samples) for the first stream
  /// @param[in] delay2 delay (in samples) for the second stream
  /// @note the buffers are treated as parts of the continuous
  /// stream. Incomplete buffers are ignored for simplicity.
  explicit SimpleCorrelator(const IndexType nDelays, 
        const IndexType delay1 = 0, const IndexType delay2 = 0) : itsDelay(delay2 - delay1), 
        itsAccumulator(nDelays * (nDelays + 1) / 2, AccType(0)), itsNDelays(nDelays) {}
  
  /// @brief reset accumulator, adjust delays
  /// @details This method is equivalent to the constructor, but it doesn't change the number
  /// of delay steps supported by the class
  /// @param[in] delay1 delay (in samples) for the first stream
  /// @param[in] delay2 delay (in samples) for the second stream
  /// @note the buffers are treated as parts of the continuous
  /// stream. Incomplete buffers are ignored for simplicity.
  void reset(const IndexType delay1, const IndexType delay2);
  
  /// @brief just reset accumulator
  /// @details This method can be used to move to the next integration cycle
  void reset();
  
  /// @brief obtain buffer
  /// @return nDelays x (nDelays + 1) /2 long vector with accumulated statistics
  inline const std::vector<AccType>& getCorrelations() const { return itsAccumulator; }

  /// @brief accumulate buffers
  /// @details 
  /// The parameter of the template is as follows.
  ///    Iter - type of the read-only random-access iterator to read the data buffer
  ///           (it could be just an ordinary pointer)
  /// @param[in] stream1 start iterator of the first stream
  /// @param[in] stream2 start iterator of the second stream
  /// @param[in] size number of samples
  template<typename Iter>
  void accumulate(const Iter stream1, const Iter stream2, const IndexType size);
    
private:
  /// @brief delay (in samples) for the second stream w.r.t. the first
  /// @details The value is negative if the first stream is delayed w.r.t the second
  IndexType itsDelay;
  
  /// @brief buffer for accumulation
  std::vector<AccType> itsAccumulator;
  
  /// @brief number of delay steps
  /// @details Although this data member is redundant, it is handy to have it to save time
  /// when this number is needed.
  IndexType itsNDelays;
};

/// @brief correlator for 3 baselines and single delay step
/// @details This is what we might need to get the performance out.
/// Template parameters:
///    AccType - type of the accumulated values (may be different from the input data type 
///              to allow overflow)
///    IndexType - type of the sample index 
/// @ingroup swcorrelator
template<typename AccType = std::complex<float>, typename IndexType = int>         
class Simple3BaselineCorrelator {
public:
  /// @brief constructor, optionally setup initial delays
  /// @details 
  /// @param[in] delay1 delay (in samples) for the first stream
  /// @param[in] delay2 delay (in samples) for the second stream
  /// @param[in] delay3 delay (in samples) for the third stream
  /// @note the buffers are treated as parts of the continuous
  /// stream. Incomplete buffers are ignored for simplicity.
  explicit Simple3BaselineCorrelator(const IndexType delay1 = 0, 
           const IndexType delay2 = 0, const IndexType delay3 = 0);
  
  /// @brief reset accumulator, adjust delays
  /// @details This method is equivalent to the constructor
  /// @param[in] delay1 delay (in samples) for the first stream
  /// @param[in] delay2 delay (in samples) for the second stream
  /// @param[in] delay3 delay (in samples) for the third stream
  /// @note the buffers are treated as parts of the continuous
  /// stream. Incomplete buffers are ignored for simplicity.
  void reset(const IndexType delay1, const IndexType delay2, const IndexType delay3);
  
  /// @brief just reset accumulator
  /// @details This method can be used to move to the next integration cycle
  void reset();
  
  /// @brief obtain buffer
  /// @return nDelays x (nDelays + 1) /2 long vector with accumulated statistics
  inline AccType getVis12() const { return itsVis12; }

  /// @brief obtain buffer
  /// @return nDelays x (nDelays + 1) /2 long vector with accumulated statistics
  inline AccType getVis13() const { return itsVis13; }

  /// @brief obtain buffer
  /// @return nDelays x (nDelays + 1) /2 long vector with accumulated statistics
  inline AccType getVis23() const { return itsVis23; }
  
  /// @return obtain number of accumulated samples
  inline IndexType nSamples12() const { return itsSamples12;}

  /// @return obtain number of accumulated samples
  inline IndexType nSamples13() const { return itsSamples13;}
  
  /// @return obtain number of accumulated samples
  inline IndexType nSamples23() const { return itsSamples23;}

  /// @brief accumulate buffers
  /// @details 
  /// The parameter of the template is as follows.
  ///    Iter - type of the read-only random-access iterator to read the data buffer
  ///           (it could be just an ordinary pointer)
  /// @param[in] stream1 start iterator of the first stream
  /// @param[in] stream2 start iterator of the second stream
  /// @param[in] stream3 start iterator of the second stream
  /// @param[in] size number of samples
  template<typename Iter>
  void accumulate(const Iter stream1, const Iter stream2, const Iter stream3, const IndexType size);
    
private:
  /// @brief delay (in samples) for the first stream
  IndexType itsDelay1;

  /// @brief delay (in samples) for the second stream
  IndexType itsDelay2;
  
  /// @brief delay (in samples) for the third stream
  IndexType itsDelay3;
  
  /// @brief accumulator for baseline 12
  AccType itsVis12;

  /// @brief accumulator for baseline 13
  AccType itsVis13;

  /// @brief accumulator for baseline 23
  AccType itsVis23;  
  
  /// @bruef number of accumulated samples for baseline 12
  IndexType itsSamples12;

  /// @bruef number of accumulated samples for baseline 13
  IndexType itsSamples13;
  
  /// @bruef number of accumulated samples for baseline 23
  IndexType itsSamples23;
  
};

} // namespace swcorrelator

} // namespace askap

#include <swcorrelator/SimpleCorrelator.tcc>

#endif // #ifndef SIMPLE_CORRELATOR_H

