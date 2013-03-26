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


#include <swcorrelator/CorrProducts.h>
#include <askap/AskapError.h>

namespace askap {

namespace swcorrelator {

/// @brief constructor 
/// @param[in] nchan number of channels (cards)
/// @param[in] beam beam number corresponding to this buffer
/// @param[in] nant number of antennas  
CorrProducts::CorrProducts(const int nchan, const int beam, const int nant) : 
      itsVisibility(nant * (nant - 1) / 2, nchan, casa::Complex(0.,0.)), 
      itsFlag(nant * (nant - 1) / 2, nchan, true), itsBeam(beam),
      itsBAT(0), itsUVW(nant * (nant - 1) / 2, 3, 0.), itsDelays(nant * (nant - 1) / 2,0.), 
      itsUVWValid(false), itsControl(nant,0u)
{
  ASKAPDEBUGASSERT(beam >= 0);
  ASKAPDEBUGASSERT(nant >= 3);
}
  
/// @brief initialise the buffer for a given beam and bat
/// @details
/// @param[in] bat time
void CorrProducts::init(const uint64_t bat)
{
  itsBAT = bat;
  itsUVW.set(0.);
  itsDelays.set(0.);
  itsUVWValid = false;
  itsFlag.set(true);
  itsVisibility.set(casa::Complex(0.,0.));
  itsControl.set(0u);
}

/// @brief baseline index for a pair of antennas
/// @details For more than 3 antennas mapping between antennas and baselines 
/// is handy to implement inside this method
/// @param[in] first index of the first antenna (0..nant-1)
/// @param[in] second index of the second antenna (0..nant-1)
/// @return baseline index (0..(nant*(nant-1)/2)-1)
/// @note an exception is thrown if there is no matching baseline (i.e. if first >= second)
int CorrProducts::baseline(const int first, const int second)
{
  ASKAPDEBUGASSERT((first >= 0) && (second >= 0));
  ASKAPCHECK(first < second, "Baseline "<<first<<" - "<<second<<" is not mapped as the first index should be less than the second");
  const int offset = (second + 1) * second / 2;
  const int baseline =  offset - first - 1;
  ASKAPDEBUGASSERT(baseline >= 0);
  return baseline;  
}

/// @brief index of the first antenna for a given baseline
/// @details It is handy to encapsulate mapping between baseline and antenna
/// indices.
/// @param[in] baseline baseline index (0..(nant*(nant-1)/2-1))
/// @return index of the first antenna
int CorrProducts::first(const int baseline)
{
   // look-up table for small number of antennas (as it is our use case realistically)
   const int ants[15] = {0, 1, 0, 2, 1, 0, 3, 2, 1, 0, 4, 3, 2, 1, 0};
   if (baseline < 15) {
       ASKAPDEBUGASSERT(baseline >= 0);
       return ants[baseline];  
   }
   
   // checks for non-zero baseline index are done inside the "second" method 
   const int ant2index = second(baseline);
   // number of baselines with antennas up to and including ant2index
   const int maxBaselines = ant2index * (ant2index + 1) / 2;
   ASKAPDEBUGASSERT(baseline < maxBaselines);
   const int result = maxBaselines - baseline - 1;
   return result;
}

/// @brief index of the second antenna for a given baseline
/// @details It is handy to encapsulate mapping between baseline and antenna
/// indices.
/// @param[in] baseline baseline index (0..(nant*(nant-1)/2-1))
/// @return index of the second antenna
int CorrProducts::second(const int baseline)
{
  ASKAPDEBUGASSERT(baseline >= 0);  
  // look-up table for small number of antennas (as it is our use case realistically)
  const int ants[15] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
    
  const int result = baseline < 15 ? ants[baseline] : int((1.+sqrt(1.+8.*baseline))/2);
  return result;
}



} // namespace swcorrelator

} // namespace askap
