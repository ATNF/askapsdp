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

// own includes
#include <dataaccess/MemBufferDataAccessor.h>

using namespace askap;
using namespace askap::accessors;


/// construct an object linked with the given const accessor
/// @param[in] acc a reference to the associated accessor
MemBufferDataAccessor::MemBufferDataAccessor(const IConstDataAccessor &acc) :
      MetaDataAccessor(acc) {}
  
/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& MemBufferDataAccessor::visibility() const
{
  resizeBufferIfNeeded();
  return itsBuffer;
}
	
/// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& MemBufferDataAccessor::rwVisibility()
{
  resizeBufferIfNeeded();
  return itsBuffer;
}

/// @brief a helper method to ensure the buffer has appropriate shape
void MemBufferDataAccessor::resizeBufferIfNeeded() const
{
  const IConstDataAccessor &acc = getROAccessor();
  if (itsBuffer.nrow() != acc.nRow() || itsBuffer.ncolumn() != acc.nChannel() ||
                                        itsBuffer.nplane() != acc.nPol()) {
      itsBuffer.resize(acc.nRow(), acc.nChannel(), acc.nPol());
  }
}


