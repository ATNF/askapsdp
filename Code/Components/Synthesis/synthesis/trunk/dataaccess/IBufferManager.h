/// @file
/// @brief An interface to generic buffer (for writing visibilities)
/// @details Read-write iterator (see IDataIterator) uses the concept
/// of buffers to store scratch data. This is an abstract interface
/// to operations with such buffers.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
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

namespace synthesis {

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

} // namespace synthesis

} // namespace askap

#endif // #ifndef I_BUFFER_MANAGER_H
