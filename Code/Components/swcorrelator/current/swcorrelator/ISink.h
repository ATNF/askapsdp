/// @file 
///
/// @brief generic interface for sink of the correlation products
/// @details One of the possible implementations is the MS writer
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

#ifndef ASKAP_SWCORRELATOR_I_SINK
#define ASKAP_SWCORRELATOR_I_SINK

// own includes
#include <swcorrelator/CorrProducts.h>

// casa includes
#include <measures/Measures/MEpoch.h>

// boost includes
#include <boost/utility.hpp>

namespace askap {

namespace swcorrelator {

/// @brief generic interface for sink of the correlation products
/// @details One of the possible implementations is the MS writer
/// @ingroup swcorrelator
struct ISink : private boost::noncopyable {
  /// @brief virtual destructor to keep the compiler happy
  virtual ~ISink() {};
  
  /// @brief calculate uvw for the given buffer
  /// @param[in] buf products buffer
  /// @note The calculation is bypassed if itsUVWValid flag is already set in the buffer
  /// @return time epoch corresponding to the BAT of the buffer
  virtual casa::MEpoch calculateUVW(CorrProducts &buf) const = 0;
  
  /// @brief write one buffer to the measurement set
  /// @details Current fieldID and dataDescID are assumed
  /// @param[in] buf products buffer
  /// @note This method could've received a const reference to the buffer. However, more
  /// workarounds would be required with casa arrays, so we don't bother doing this at the moment.
  /// In addition, we could call calculateUVW inside this method (but we still need an option to
  /// calculate uvw's ahead of writing the buffer if we implement some form of delay tracking).
  virtual void write(CorrProducts &buf) const = 0;
};
  
} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_I_SINK

