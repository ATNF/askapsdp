/// @file 
/// @brief Implementation of IDataSource in the table-based case
/// @details
/// TableDataSource: Allow read-write access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
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

#ifndef TABLE_DATA_SOURCE_H
#define TABLE_DATA_SOURCE_H

#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/IDataSource.h>

namespace askap {

namespace synthesis {

/// @brief Implementation of IDataSource in the table-based case
/// @details
/// TableDataSource: Allow read-write access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
/// @ingroup dataaccess_tab
class TableDataSource : public TableConstDataSource,
                        virtual public IDataSource,
			virtual protected TableInfoAccessor
{
public:
  /// various options affecting the behavior of this DataSource
  enum TableDataSourceOptions {
     /// default operations, i.e. subtable-based buffers, use existing
     /// buffers if available
     DEFAULT = 0,
     /// delete BUFFERS subtable in the measurement set, if present
     REMOVE_BUFFERS = 1,
     /// create buffers in memory (via MemoryTable)
     MEMORY_BUFFERS = 2,
     /// allow to write to the measurement set
     WRITE_PERMITTED = 4
  };
  
  /// construct a read-write data source object
  /// @param[in] fname file name of the measurement set to use
  /// @param[in] opt options from TableDataSourceOptions, can be or'ed
  /// @param[in] dataColumn a name of the data column used by default
  ///                       (default is DATA)
  explicit TableDataSource(const std::string &fname, int opt =
              TableDataSource::DEFAULT, const std::string &dataColumn = "DATA");

  /// @brief obtain a read/write iterator
  /// @details 
  /// get a read/write iterator over a selected part of the dataset 
  /// represented by this DataSource object with an explicitly 
  /// specified conversion policy. This is the most general 
  /// createIterator(...) call, which
  /// is used as a default implementation for all less general cases
  /// (although they can be overriden in the derived classes, if it 
  ///  will be necessary because of the performance issues)
  ///
  /// @param[in] sel a shared pointer to the selector object defining 
  ///            which subset of the data is used
  /// @param[in] conv a shared pointer to the converter object defining
  ///            reference frames and units to be used
  /// @return a shared pointer to DataIterator object
  /// @note
  /// The method acts as a factory by creating a new DataIterator.
  /// The lifetime of this iterator is the same as the lifetime of
  /// the DataSource object. Therefore, it can be reused multiple times,
  /// if necessary. Call init() to rewind the iterator.
  virtual boost::shared_ptr<IDataIterator> createIterator(const
             IDataSelectorConstPtr &sel, const
  	   IDataConverterConstPtr &conv) const;
  	   
  // we need this to get access to the overloaded syntax in the base class 
  using IDataSource::createIterator;	   
};
 
} // namespace synthesis

} // namespace askap

#endif // #ifndef TABLE_DATA_SOURCE_H
