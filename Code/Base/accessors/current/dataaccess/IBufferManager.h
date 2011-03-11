/// @file
/// @brief An interface to generic buffer (for writing visibilities)
/// @details Read-write iterator (see IDataIterator) uses the concept
/// of buffers to store scratch data. This is an abstract interface
/// to operations with such buffers.
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

#ifndef I_BUFFER_MANAGER_H
#define I_BUFFER_MANAGER_H

// std includes
#include <string>

// own includes
#include <dataaccess/IHolder.h>

// casa includes
#include <casa/Arrays/Cube.h>
#include <casa/BasicSL/Complex.h>

namespace askap {

namespace accessors {

/// @brief An interface to generic buffer (for writing visibilities)
/// @details Read-write iterator (see IDataIterator) uses the concept
/// of buffers to store scratch data. This is an abstract interface
/// to operations with such buffers.
/// @ingroup dataaccess_hlp
struct IBufferManager : virtual public IHolder
{
  /// @brief populate the cube with the data stored in the given buffer
  /// @details The method throws an exception if the requested buffer
  /// does not exist (prevents a shape mismatch)
  /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the complex visibility data
  /// @param[in] name a name of the buffer to work with
  /// @param[in] index a sequential index in the buffer
  virtual void readBuffer(casa::Cube<casa::Complex> &vis,
                          const std::string &name,
			  casa::uInt index) const = 0;
  
  /// @brief write the cube back to the given buffer
  /// @details This buffer is created on the first write operation
  /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the complex visibility data
  /// @param[in] name a name of the buffer to work with
  /// @param[in] index a sequential index in the buffer
  virtual void writeBuffer(const casa::Cube<casa::Complex> &vis,
                           const std::string &name,
			   casa::uInt index) const = 0;

  /// @brief check whether the particular buffer exists
  /// @param[in] name a name of the buffer to query
  /// @param[in] index a sequential index in the buffer
  /// @return true, if the buffer with the given name is present
  virtual bool bufferExists(const std::string &name,
			   casa::uInt index) const = 0;
};

} // namespace accessors

} // namespace askap

#endif // #ifndef I_BUFFER_MANAGER_H
