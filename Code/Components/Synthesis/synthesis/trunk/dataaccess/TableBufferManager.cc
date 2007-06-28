/// @file
/// @brief A class to manage buffers stored in subtable
/// @details Read-write iterator (see IDataIterator) uses the concept
/// of buffers to store scratch data. This class stores buffers in the
/// BUFFERS subtable 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/TableBufferManager.h>

using namespace conrad;
using namespace synthesis;

/// construct the object and link it to the given buffers subtable
/// @param[in] tab  subtable to use
TableBufferManager::TableBufferManager(const casa::Table &tab) :
                   TableHolder(tab) {}


/// @brief populate the cube with the data stored in the given buffer
/// @details The method throws an exception if the requested buffer
/// does not exist (prevents a shape mismatch)
/// @param[in] vis a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the complex visibility data
/// @param[in] name a name of the buffer to work with
/// @param[in] index a sequential index in the buffer
void TableBufferManager::readBuffer(casa::Cube<casa::Complex> &vis,
                        const std::string &name,
			   casa::uInt index) const
{
}

/// @brief write the cube back to the given buffer
/// @details This buffer is created on the first write operation
/// @param[in] vis a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the complex visibility data
/// @param[in] name a name of the buffer to work with
/// @param[in] index a sequential index in the buffer
void TableBufferManager::writeBuffer(const casa::Cube<casa::Complex> &vis,
                         const std::string &name,
			   casa::uInt index) const
{
}

/// @brief check whether the particular buffer exists
/// @param[in] name a name of the buffer to query
/// @param[in] index a sequential index in the buffer
/// @return true, if the buffer with the given name is present
bool TableBufferManager::bufferExists(const std::string &name,
			   casa::uInt index) const
{
  return false;
}
