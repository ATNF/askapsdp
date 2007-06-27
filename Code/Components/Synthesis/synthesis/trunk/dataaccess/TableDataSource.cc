/// @file 
/// @brief Implementation of IDataSource in the table-based case
/// @details
/// TableDataSource: Allow read-write access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// casa includes

// own includes
#include <dataaccess/TableDataSource.h>
#include <dataaccess/TableDataIterator.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/IDataConverterImpl.h>
#include <dataaccess/ITableDataSelectorImpl.h>

using namespace conrad;
using namespace synthesis;

/// construct a read-write data source object
/// @param[in] fname file name of the measurement set to use
///
TableDataSource::TableDataSource(const std::string &fname)  :
         TableInfoAccessor(casa::Table(fname,casa::Table::Update)) {}

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
boost::shared_ptr<IDataIterator> TableDataSource::createIterator(const
           IDataSelectorConstPtr &sel, const
	   IDataConverterConstPtr &conv) const
{
 // cast input selector to "implementation" interface
   boost::shared_ptr<ITableDataSelectorImpl const> implSel=
           boost::dynamic_pointer_cast<ITableDataSelectorImpl const>(sel);
   boost::shared_ptr<IDataConverterImpl const> implConv=
           boost::dynamic_pointer_cast<IDataConverterImpl const>(conv);
   	   
   if (!implSel || !implConv) {
       CONRADTHROW(DataAccessLogicError, "Incompatible selector and/or "<<
                 "converter are received by the createIterator method");
   }
   return boost::shared_ptr<IDataIterator>(new TableDataIterator(
                getTableManager(),implSel,implConv)); 
}
