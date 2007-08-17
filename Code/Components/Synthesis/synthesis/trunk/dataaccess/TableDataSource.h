/// @file 
/// @brief Implementation of IDataSource in the table-based case
/// @details
/// TableDataSource: Allow read-write access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_DATA_SOURCE_H
#define TABLE_DATA_SOURCE_H

#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/IDataSource.h>

namespace conrad {

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
  TableDataSource(const std::string &fname, int opt =
                                            TableDataSource::DEFAULT);

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

} // namespace conrad

#endif // #ifndef TABLE_DATA_SOURCE_H
