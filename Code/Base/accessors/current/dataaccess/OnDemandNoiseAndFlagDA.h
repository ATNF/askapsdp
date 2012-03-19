/// @file
/// @brief an adapter allowing on demand substitution of noise and flag cubes
/// @details This class extends further MemBufferDataAccessor adapter by
/// providing intefaces to update noise and  flagging information.
/// By default the original metadata are returned by the noise() and flag() methods.
/// However, at the first call to rwNoise or rwFlag, a copy has been created
/// for the appropriate cube and returned for an optional modification. Then, this
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

#ifndef ON_DEMAND_NOISE_AND_FLAG_DA_H
#define ON_DEMAND_NOISE_AND_FLAG_DA_H

// own includes
#include <dataaccess/MemBufferDataAccessor.h>
#include <dataaccess/IFlagAndNoiseDataAccessor.h>


namespace askap {
	
namespace accessors {

/// @brief an adapter allowing on demand substitution of noise and flag cubes
/// @details This class extends further MemBufferDataAccessor adapter by
/// providing intefaces to update noise and  flagging information.
/// By default the original metadata are returned by the noise() and flag() methods.
/// However, at the first call to rwNoise or rwFlag, a copy has been created
/// for the appropriate cube and returned for an optional modification. Then, this
/// copied cube is returned by the read-only methods.
/// @ingroup dataaccess_hlp
class OnDemandNoiseAndFlagDA : virtual public MemBufferDataAccessor,
                               virtual public IFlagAndNoiseDataAccessor
{
public:
  /// construct an object linked with the given const accessor
  /// @param[in] acc a reference to the associated accessor
  explicit OnDemandNoiseAndFlagDA(const IConstDataAccessor &acc);
  
  /// @brief Noise level required for a proper weighting
  /// @return a reference to nRow x nChannel x nPol cube with
  ///         complex noise estimates. Elements correspond to the
  ///         visibilities in the data cube.
  virtual const casa::Cube<casa::Complex>& noise() const;

  /// @brief write access to Noise level 
  /// @return a reference to nRow x nChannel x nPol cube with
  ///         complex noise estimates. Elements correspond to the
  ///         visibilities in the data cube.
  virtual casa::Cube<casa::Complex>& rwNoise();
  
  /// @brief Cube of flags corresponding to the output of visibility()
  /// @return a reference to nRow x nChannel x nPol cube with the flag
  ///         information. If True, the corresponding element is flagged.
  virtual const casa::Cube<casa::Bool>& flag() const;

  /// @brief Non-const access to the cube of flags.
  /// @return a reference to nRow x nChannel x nPol cube with the flag
  ///         information. If True, the corresponding element is flagged.
  virtual casa::Cube<casa::Bool>& rwFlag();
  
private:  
  /// @brief if true, the flag buffer is to be used instead of metadata
  bool itsFlagSubstituted;
  
  /// @brief if true, the noise buffer is to be used instead of metadata
  bool itsNoiseSubstituted;

  /// @brief buffer for noise (used if itsNoiseSubstituted is true)
  casa::Cube<casa::Bool> itsFlagBuffer;  
  
  /// @brief buffer for noise (used if itsNoiseSubstituted is true)
  casa::Cube<casa::Complex> itsNoiseBuffer;   
};

} // namespace accessors

} // namespace askap


#endif // #ifndef ON_DEMAND_NOISE_AND_FLAG_DA_H

