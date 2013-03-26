/// @file 
///
/// @brief Final products of correlation
/// @details This class encapsulates the data, which is the final product of correlation, i.e
/// visibilities for all spectral channels and baselines, flagging information and, BAT and uvw.
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

#ifndef ASKAP_SWCORRELATOR_CORR_PRODUCTS
#define ASKAP_SWCORRELATOR_CORR_PRODUCTS

#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Constants.h>
#include <inttypes.h>

#include <boost/utility.hpp>

namespace askap {

namespace swcorrelator {

/// @brief Final products of correlation
/// @details This class encapsulates the data, which is the final product of correlation, i.e
/// visibilities for all spectral channels and baselines, flagging information and, BAT and uvw.
/// @ingroup swcorrelator
struct CorrProducts : private boost::noncopyable {
  // baselines are hardcoded at the moment in the order 1-2, 2-3 and 1-3
  
  /// @brief constructor 
  /// @param[in] nchan number of channels (cards)
  /// @param[in] beam beam number corresponding to this buffer
  /// @param[in] nant number of antennas
  CorrProducts(const int nchan, const int beam, const int nant = 3);
  
  /// @brief initialise the buffer for a given beam and bat
  /// @details
  /// @param[in] bat time
  void init(const uint64_t bat);
  
  /// @brief baseline index for a pair of antennas
  /// @details For more than 3 antennas mapping between antennas and baselines 
  /// is handy to implement inside this method
  /// @param[in] first index of the first antenna (0..nant-1)
  /// @param[in] second index of the second antenna (0..nant-1)
  /// @return baseline index (0..(nant*(nant-1)/2)-1)
  /// @note an exception is thrown if there is no matching baseline (i.e. if first >= second)
  static int baseline(const int first, const int second);
  
  /// @brief index of the first antenna for a given baseline
  /// @details It is handy to encapsulate mapping between baseline and antenna
  /// indices.
  /// @param[in] baseline baseline index (0..(nant*(nant-1)/2-1))
  /// @return index of the first antenna
  static int first(const int baseline);

  /// @brief index of the second antenna for a given baseline
  /// @details It is handy to encapsulate mapping between baseline and antenna
  /// indices.
  /// @param[in] baseline baseline index (0..(nant*(nant-1)/2-1))
  /// @return index of the second antenna
  static int second(const int baseline);
  
  
  /// @brief visibility buffer (dimensions are baseline and channel)
  casa::Matrix<casa::Complex> itsVisibility;   
  
  /// @brief flagging information (dimensions are baseline and channel)
  casa::Matrix<casa::Bool> itsFlag;
  
  /// @brief beam index (negative value means that this buffer is not valid)
  int itsBeam;
  
  /// @brief time
  uint64_t itsBAT;
  
  /// @brief baseline spacings for all 3 baselines (rows are baselines)
  casa::Matrix<double> itsUVW;
  
  /// @brief delay vector for all 3 baselines
  /// @details We can't use W from itsUVW because it is in J2000 rather than JTRUE
  casa::Vector<double> itsDelays;
  
  /// @brief flag that uvw matrix and delay vector are filled with valid info
  bool itsUVWValid;

  /// @brief user defined control words for antennas 1,2 and 3
  casa::Vector<uint32_t> itsControl;
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_CORR_PRODUCTS



