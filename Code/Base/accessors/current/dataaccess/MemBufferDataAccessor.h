/// @file
/// @brief an adapter to most methods of IConstDataAccessor
///
/// @details It is sometimes necessary to use a simple cube instead of
/// the full functionality of buffers provided by the read-write accessor.
/// Typically, the need for such class arises if one needs a buffering
/// on each individual iteration and the content of buffers is not required 
/// to be preserved when the corresponding iterator advances. In most cases,
/// a casa::Cube with the same dimensions as that returned by the visibility
/// method can be used. However, it can also be desireable to be able to
/// use existing API accepting a reference to an accessor for this buffer, or,
/// alternatively to pass around this buffer with associated metadata supplied
/// by the original accessor. This adapter can help in both situations.
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

#ifndef MEM_BUFFER_DATA_ACCESSOR_H
#define MEM_BUFFER_DATA_ACCESSOR_H

// own includes
#include <dataaccess/MetaDataAccessor.h>
#include <dataaccess/IDataAccessor.h>


namespace askap {
	
namespace accessors {

/// @brief an adapter to most methods of IConstDataAccessor
///
/// @details It is sometimes necessary to use a simple cube instead of
/// the full functionality of buffers provided by the read-write accessor.
/// Typically, the need for such class arises if one needs a buffering
/// on each individual iteration and the content of buffers is not required 
/// to be preserved when the corresponding iterator advances. In most cases,
/// a casa::Cube with the same dimensions as that returned by the visibility
/// method can be used. However, it can also be desireable to be able to
/// use existing API accepting a reference to an accessor for this buffer, or,
/// alternatively to pass around this buffer with associated metadata supplied
/// by the original accessor. This adapter can help in both situations.
/// It acts as a read-only accessor supplied at the construction stage for
/// all metadata requests and returns a reference to the internal buffer for
/// both read-only and read-write visibility access methods (the buffer is
/// resized automatically to match the cube provided by the accessor). 
/// @ingroup dataaccess_hlp
class MemBufferDataAccessor : virtual public MetaDataAccessor,
                              virtual public IDataAccessor
{
public:
  /// construct an object linked with the given const accessor
  /// @param[in] acc a reference to the associated accessor
  explicit MemBufferDataAccessor(const IConstDataAccessor &acc);
  
  /// Read-only visibilities (a cube is nRow x nChannel x nPol; 
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual const casa::Cube<casa::Complex>& visibility() const;

	
  /// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual casa::Cube<casa::Complex>& rwVisibility();
  
private:
  /// @brief a helper method to ensure the buffer has appropriate shape
  void resizeBufferIfNeeded() const;
  
  /// @brief actual buffer
  mutable casa::Cube<casa::Complex> itsBuffer;
};

} // namespace accessors

} // namespace askap


#endif // #ifndef MEM_BUFFER_DATA_ACCESSOR_H
