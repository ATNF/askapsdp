/// @file
/// @brief an adapter allowing on demand substitution of frequency and (in future) uvw
/// @details This class extends further OnDemandNoiseAndFlagDA adapter by
/// providing intefaces to update frequency and (in future) uvw.
/// By default the original metadata are returned by the frequency() method.
/// However, at the first call to rwFrequency, a copy has been created
/// for the appropriate vector and returned for an optional modification. Then, this
/// copied cube is returned by the read-only methods.
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

#include <dataaccess/SmearingAccessorAdapter.h>
#include <askap/AskapError.h>

namespace askap {

namespace accessors {

/// construct an object linked with the given const accessor
/// @param[in] acc a reference to the associated accessor
SmearingAccessorAdapter::SmearingAccessorAdapter(const IConstDataAccessor &acc) :
   MetaDataAccessor(acc), MemBufferDataAccessor(acc), OnDemandNoiseAndFlagDA(acc), 
   itsFrequencySubstituted(false) {}

/// Frequency for each channel
/// @return a reference to vector containing frequencies for each
///         spectral channel (vector size is nChannel). Frequencies
///         are given as Doubles, the frame/units are specified by
///         the DataSource object
const casa::Vector<casa::Double>& SmearingAccessorAdapter::frequency() const
{
  return itsFrequencySubstituted ? itsFrequencyBuffer : getROAccessor().frequency();
}

/// @brief read-write access to the frequency
/// @details The first call to this method detaches the adapter from the original
/// metadata and returns the reference to the buffer with a copy of the original frequencies.
/// All subsequent calls to read-only or read-write access methods work with the buffer.
/// @return a reference to vector containing frequencies for each
///         spectral channel (vector size is nChannel). Frequencies
///         are given as Doubles, the frame/units are specified by
///         the DataSource object
casa::Vector<casa::Double>& SmearingAccessorAdapter::rwFrequency()
{
  if (!itsFrequencySubstituted) {
      itsFrequencySubstituted = true;
      itsFrequencyBuffer.assign(getROAccessor().frequency().copy());
  }
  return itsFrequencyBuffer;  
}
  
/// @brief force the adapter to use the buffer
/// @details This method matches well the intended use case of this adapter. It
/// just detaches the adapter from the original metadata and resizes the buffer
/// (but doesn't copy the data)
void SmearingAccessorAdapter::useFrequencyBuffer()
{
  // this method is supposed to be used with a detached adapter only
  ASKAPDEBUGASSERT(!itsFrequencySubstituted);
  itsFrequencySubstituted = true;
  itsFrequencyBuffer.resize(getROAccessor().nChannel()); 
}


} // namespace accessors

} // namespace askap