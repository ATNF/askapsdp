/// @file TableConstDataSource.h
/// @brief Implementation of IConstDataSource in the table-based case
/// @details
/// TableConstDataSource: Allow read-only access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_CONST_DATA_SOURCE_H
#define TABLE_CONST_DATA_SOURCE_H

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/IConstDataSource.h>
#include <dataaccess/TableInfoAccessor.h>

// std includes
#include <string>

namespace conrad {

namespace synthesis {

/// @brief Implementation of IConstDataSource in the table-based case
/// @details
/// TableConstDataSource: Allow read-only access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
/// @ingroup dataaccess_tab
class TableConstDataSource : virtual public IConstDataSource,
                             virtual protected TableInfoAccessor
{
public:
  /// construct a read-only data source object
  /// @param[in] fname file name of the measurement set to use
  ///
  TableConstDataSource(const std::string &fname);
  
  /// create a converter object corresponding to this type of the
  /// DataSource. The user can change converting policies (units,
  /// reference frames) by appropriate calls to this converter object
  /// and pass it back to createConstIterator(...). The data returned by
  /// the iteratsr will automatically be in the requested frame/units
  ///
  /// @return a shared pointer to a new DataConverter object
  ///
  /// The method acts as a factory by creating a new DataConverter.
  /// The lifetime of this converter is the same as the lifetime of the
  /// DataSource object. Therefore, it can be reused multiple times,
  /// if necessary. However, the behavior of iterators created
  /// with a particular DataConverter is undefined, if you change
  /// the DataConverter after the creation of an iterator, unless you
  /// call init() of the iterator (and start a new iteration loop).
  virtual IDataConverterPtr createConverter() const;
  
  /// get iterator over a selected part of the dataset represented
  /// by this DataSource object with an explicitly specified conversion
  /// policy. This is the most general createConstIterator(...) call, 
  /// which is used as a default implementation for all less general 
  /// cases (although they can be overriden in the derived classes, if it 
  /// will be necessary because of the performance issues)
  ///
  /// @param[in] sel a shared pointer to the selector object defining 
  ///            which subset of the data is used
  /// @param[in] conv a shared pointer to the converter object defining
  ///            reference frames and units to be used
  /// @return a shared pointer to DataIterator object
  ///
  /// The method acts as a factory by creating a new DataIterator.
  /// The lifetime of this iterator is the same as the lifetime of
  /// the DataSource object. Therefore, it can be reused multiple times,
  /// if necessary. Call init() to rewind the iterator.
  virtual boost::shared_ptr<IConstDataIterator> createConstIterator(const
             IDataSelectorConstPtr &sel,
             const IDataConverterConstPtr &conv) const;
  
  // we need this to get access to the overloaded syntax in the base class 
  using IConstDataSource::createConstIterator;
 
  /// create a selector object corresponding to this type of the
  /// DataSource
  ///
  /// @return a shared pointer to the DataSelector corresponding to
  /// this type of DataSource. DataSource acts as a factory and
  /// creates a selector object of the appropriate type
  ///
  /// This method acts as a factory by creating a new DataSelector
  /// appropriate to the given DataSource. The lifetime of the
  /// DataSelector is the same as the lifetime of the DataSource 
  /// object. Therefore, it can be reused multiple times,
  /// if necessary. However, the behavior of iterators already obtained
  /// with this DataSelector is undefined, if one changes the selection
  /// unless the init method is called for the iterator (and the new
  /// iteration loop is started).
  virtual IDataSelectorPtr createSelector() const;
protected:
  /// construct a part of the read only object for use in the
  /// derived classes
  TableConstDataSource();
  
};
 
} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_CONST_DATA_SOURCE_H
