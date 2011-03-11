/// @file
/// @brief an adapter to most methods of IConstDataAccessor with buffering
///
/// @details This class is somewhat similar to MemBufferDataAccessor, however it is
/// not as basic. The latter doesn't manage the cube at all and only ensures that it has
/// a conforming size. In contrast, this class returns the existing read-only visibility cube 
/// until a non-const reference is requested (rwVisibility). Then the read-only visibilities are
/// copied to the internal buffer and the reference to this buffer is passed for all later calls
/// to read-write and read-only methods until either the shape changes or discardCache method is
/// called. The intention is to provide a similar functionality for the flagging methos
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
#include <dataaccess/OnDemandBufferDataAccessor.h>

using namespace askap;
using namespace askap::accessors;


/// construct an object linked with the given const accessor
/// @param[in] acc a reference to the associated accessor
OnDemandBufferDataAccessor::OnDemandBufferDataAccessor(const IConstDataAccessor &acc) :
      MetaDataAccessor(acc), itsUseBuffer(false) {}
  
/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casa::Cube<casa::Complex>& OnDemandBufferDataAccessor::visibility() const
{
  checkBufferSize();
  if (itsUseBuffer) {
      return itsBuffer;
  }
  return getROAccessor().visibility();
}
	
/// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& OnDemandBufferDataAccessor::rwVisibility()
{
  checkBufferSize();
  if (!itsUseBuffer) {
      itsBuffer = getROAccessor().visibility().copy();
      itsUseBuffer = true;
  }
  return itsBuffer;  
}

/// @brief a helper method to check whether the buffer has a correct size
/// @details The wrong size means that the iterator has advanced and this
/// accessor has to be coupled back to the read-only accessor which has been given at the 
/// construction. If a wrong size is detected, itsUseBuffer flag is reset.
void OnDemandBufferDataAccessor::checkBufferSize() const
{
  const IConstDataAccessor &acc = getROAccessor();
  if (itsBuffer.nrow() != acc.nRow() || itsBuffer.ncolumn() != acc.nChannel() ||
                                        itsBuffer.nplane() != acc.nPol()) {
      // couple the class to the original accessor
      // discardCache operates with just mutable data members. Although technically discardCache
      // can be made a const method, it is probably conceptually wrong. Therefore, we take the
      // constness out, instead.
      const_cast<OnDemandBufferDataAccessor*>(this)->discardCache(); 
  }
}

/// @brief discard the content of the cache
/// @details A call to this method would switch the accessor to the pristine state
/// it had straight after construction. A new call to rwVisibility would be required 
/// to decouple from the read-only accessor 
void OnDemandBufferDataAccessor::discardCache()
{
  itsUseBuffer = false;
  itsBuffer.resize(0,0,0);
}

