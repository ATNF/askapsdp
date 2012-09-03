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
CorrProducts::CorrProducts(const int nchan, const int beam) : itsVisibility(3, nchan, casa::Complex(0.,0.)), 
      itsFlag(3, nchan, true), itsBeam(beam),
      itsBAT(0), itsUVW(3, 3, 0.), itsDelays(3,0.), itsUVWValid(false) 
{
  ASKAPDEBUGASSERT(beam >= 0);
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
}

} // namespace swcorrelator

} // namespace askap
