/// @file TableConstDataSource.cc
/// @brief Implementation of IConstDataSource in the table-based case
/// @details
/// TableConstDataSource: Allow read-only access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// boost includes
#include <boost/shared_ptr.hpp>

/// own includes
#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/TableConstDataIterator.h>
#include <dataaccess/TableDataSelector.h>
#include <dataaccess/BasicDataConverter.h>
#include <dataaccess/DataAccessError.h>

using namespace conrad;
using namespace conrad::synthesis;
using namespace casa;

/// construct a read-only data source object
/// @param[in] fname file name of the measurement set to use
///
TableConstDataSource::TableConstDataSource(const std::string &fname) :
         TableInfoAccessor(casa::Table(fname)) {}

/// construct a part of the read only object for use in the
/// derived classes
/// @note Due to virtual inheritance, TableInfoAccessor will be initialized
/// in the concrete derived class. This empty constructor is added to make
/// the compiler happy
TableConstDataSource::TableConstDataSource() :
         TableInfoAccessor(boost::shared_ptr<ITableManager const>()) {} 

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
IDataConverterPtr TableConstDataSource::createConverter() const
{
  return IDataConverterPtr(new BasicDataConverter);
}

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
boost::shared_ptr<IConstDataIterator>
TableConstDataSource::createConstIterator(const IDataSelectorConstPtr &sel,
              const IDataConverterConstPtr &conv) const
{
   // cast input selector to "implementation" interface
   boost::shared_ptr<ITableDataSelectorImpl const> implSel=
           boost::dynamic_pointer_cast<ITableDataSelectorImpl const>(sel);
   boost::shared_ptr<IDataConverterImpl const> implConv=
           boost::dynamic_pointer_cast<IDataConverterImpl const>(conv);
   	   
   if (!implSel || !implConv) {
       CONRADTHROW(DataAccessLogicError, "Incompatible selector and/or "<<
                 "converter are received by the createConstIterator method");
   }
   return boost::shared_ptr<IConstDataIterator>(new TableConstDataIterator(
                getTableManager(),implSel,implConv));
}

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
IDataSelectorPtr TableConstDataSource::createSelector() const
{
  return IDataSelectorPtr(new TableDataSelector(getTableManager()));
}

