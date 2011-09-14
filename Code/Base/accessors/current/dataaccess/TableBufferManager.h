/// @file
/// @brief A class to manage buffers stored in subtable
/// @details Read-write iterator (see IDataIterator) uses the concept
/// of buffers to store scratch data. This class stores buffers in the
/// BUFFERS subtable 
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

#ifndef TABLE_BUFFER_MANAGER_H
#define TABLE_BUFFER_MANAGER_H

#include <dataaccess/IBufferManager.h>
#include <dataaccess/TableHolder.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Cube.h>


namespace askap {

namespace accessors {

/// @brief A class to manage buffers stored in subtable
/// @details Read-write iterator (see IDataIterator) uses the concept
/// of buffers to store scratch data. This class stores buffers in the
/// BUFFERS subtable 
/// @ingroup dataaccess_tab
struct TableBufferManager : virtual public IBufferManager,
                            virtual protected TableHolder
{
  /// construct the object and link it to the given buffers subtable
  /// @param[in] tab  subtable to use
  TableBufferManager(const casa::Table &tab);
  
  /// @brief populate the cube with the data stored in the given buffer
  /// @details The method throws an exception if the requested buffer
  /// does not exist (prevents a shape mismatch)
  /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the complex visibility data
  /// @param[in] name a name of the buffer to work with
  /// @param[in] index a sequential index in the buffer
  virtual void readBuffer(casa::Cube<casa::Complex> &vis,
                          const std::string &name,
			   casa::uInt index) const;
  
  /// @brief write the cube back to the given buffer
  /// @details This buffer is created on the first write operation
  /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the complex visibility data
  /// @param[in] name a name of the buffer to work with
  /// @param[in] index a sequential index in the buffer
  virtual void writeBuffer(const casa::Cube<casa::Complex> &vis,
                           const std::string &name,
			   casa::uInt index) const;

  /// @brief check whether the particular buffer exists
  /// @param[in] name a name of the buffer to query
  /// @return true, if the buffer with the given name is present
  /// @param[in] index a sequential index in the buffer
  virtual bool bufferExists(const std::string &name,
			   casa::uInt index) const;
protected:
  // templated methods to handle cubes of different types

  /// @brief populate the cube with the data stored in the given table cell
  /// @details The method throws an exception if the requested table cell
  /// does not exist
  /// @param[in] cube a reference to a cube of some type
  /// @param[in] name a name of the column to work with
  /// @param[in] index row number
  template<typename T>
  void readCube(casa::Cube<T> &cube, const std::string &name,
			    casa::uInt index) const;
  
  /// @brief write the cube back to the table
  /// @details The table cell is populated with values on the first write 
  /// operation
  /// @param[in] cube to take the data from 
  /// @param[in] name a name of the column to work with
  /// @param[in] index row number
  template<typename T>
  void writeCube(const casa::Cube<T> &cube, const std::string &name,
			     casa::uInt index) const;

  /// @brief check whether a particular table cell exists
  /// @param[in] name a name of the table column to query
  /// @param[in] index row number 
  /// @return true, if the given cell exists and has an array
  /// @note template type defined the type of the data
  template<typename T>
  bool cellDefined(const std::string &name,
			      casa::uInt index) const;  			   
};

} // namespace accessors

} // namespace askap

#include <dataaccess/TableBufferManager.tcc>

#endif // #ifndef TABLE_BUFFER_MANAGER_H
