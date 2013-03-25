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

/// @brief baseline index for pairs of antennas
/// @details For more than 3 antennas mapping between antennas and baselines 
/// is handy to implement inside this method
/// @param[in] first index of the first antenna (0..nant-1)
/// @param[in] second index of the second antenna (0..nant-1)
/// @return baseline index (0..(nant*(nant-1)/2))
/// @note an exception is thrown if there is no matching baseline
int CorrProducts::baseline(const int first, const int second) const
{
  ASKAPDEBUGASSERT((first >= 0) && (second >= 0));
  ASKAPCHECK(first < int(itsControl.nelements()), "First antenna index is out of range, have only "<<
             itsControl.nelements()<<" antennas");
  ASKAPCHECK(second < int(itsControl.nelements()), "Second antenna index is out of range, have only "<<
             itsControl.nelements()<<" antennas");
  ASKAPCHECK(first<second, "Baseline "<<first<<" - "<<second<<" is not mapped as first index should be less than second");
  const int offset = (second + 1) * second / 2;
  const int baseline =  offset - first - 1;
  ASKAPDEBUGASSERT(baseline >= 0);
  return baseline;  
}


} // namespace swcorrelator

} // namespace askap
