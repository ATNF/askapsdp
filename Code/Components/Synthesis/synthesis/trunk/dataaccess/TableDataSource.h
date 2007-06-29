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

class TableDataSource : public TableConstDataSource,
                        virtual public IDataSource,
			virtual protected TableInfoAccessor
{
public:
  /// construct a read-write data source object
  /// @param[in] fname file name of the measurement set to use
  /// @param[in] newBuffers, if True the BUFFERS subtable will be
  /// removed, if it already exists.   
  TableDataSource(const std::string &fname, bool newBuffers=false);

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
};
 
} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_DATA_SOURCE_H
