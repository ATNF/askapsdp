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

#ifndef SMEARING_ACCESSOR_ADAPTER_H
#define SMEARING_ACCESSOR_ADAPTER_H

// own includes
#include <dataaccess/OnDemandNoiseAndFlagDA.h>
#include <dataaccess/IConstDataAccessor.h>

// casa includes
#include <casa/Arrays/Vector.h>

namespace askap {
	
namespace accessors {

/// @brief an adapter allowing on demand substitution of frequency and (in future) uvw
/// @details This class extends further OnDemandNoiseAndFlagDA adapter by
/// providing intefaces to update frequency and (in future) uvw.
/// By default the original metadata are returned by the frequency() method.
/// However, at the first call to rwFrequency, a copy has been created
/// for the appropriate vector and returned for an optional modification. Then, this
/// copied cube is returned by the read-only methods.
/// @ingroup dataaccess_hlp
class SmearingAccessorAdapter : virtual public OnDemandNoiseAndFlagDA
{
public:
  /// construct an object linked with the given const accessor
  /// @param[in] acc a reference to the associated accessor
  explicit SmearingAccessorAdapter(const IConstDataAccessor &acc);

  /// Frequency for each channel
  /// @return a reference to vector containing frequencies for each
  ///         spectral channel (vector size is nChannel). Frequencies
  ///         are given as Doubles, the frame/units are specified by
  ///         the DataSource object
  virtual const casa::Vector<casa::Double>& frequency() const;

  /// @brief read-write access to the frequency
  /// @details The first call to this method detaches the adapter from the original
  /// metadata and returns the reference to the buffer with a copy of the original frequencies.
  /// All subsequent calls to read-only or read-write access methods work with the buffer.
  /// @return a reference to vector containing frequencies for each
  ///         spectral channel (vector size is nChannel). Frequencies
  ///         are given as Doubles, the frame/units are specified by
  ///         the DataSource object
  virtual casa::Vector<casa::Double>& rwFrequency();
  
  /// @brief force the adapter to use the buffer
  /// @details This method matches well the intended use case of this adapter. It
  /// just detaches the adapter from the original metadata and resizes the buffer
  /// (but doesn't copy the data)
  void useFrequencyBuffer();

private:  
  /// @brief if true, the frequency buffer is to be used instead of the original metadata
  bool itsFrequencySubstituted;
  
  /// @brief buffer for noise (used if itsNoiseSubstituted is true)
  casa::Vector<casa::Double> itsFrequencyBuffer;  
};

} // namespace accessors

} // namespace askap

#endif // #ifndef SMEARING_ACCESSOR_ADAPTER_H

