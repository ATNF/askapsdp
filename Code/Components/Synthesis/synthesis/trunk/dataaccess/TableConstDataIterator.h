/// @file TableConstDataIterator.h
///
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// This is implementation in the table-based case.
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_CONST_DATA_ITERATOR_H
#define TABLE_CONST_DATA_ITERATOR_H

/// boost includes
#include <boost/shared_ptr.hpp>

/// casa includes
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>

/// own includes
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataConverterImpl.h>
#include <dataaccess/ITableDataSelectorImpl.h>
#include <dataaccess/DataAccessorStub.h>

namespace conrad {

namespace synthesis {

class TableConstDataIterator : virtual public IConstDataIterator
{
public:
  /// @param[in] ms the measurement set to use (as a reference to table)
  /// @param[in] sel shared pointer to selector
  /// @param[in] conv shared pointer to converter
  /// @param[in] maxChunkSize maximum number of rows per accessor
  TableConstDataIterator(const casa::Table &ms,
              const boost::shared_ptr<ITableDataSelectorImpl const> &sel,
	      const boost::shared_ptr<IDataConverterImpl const> &conv,
	      casa::uInt maxChunkSize = MAXINT);

  /// Restart the iteration from the beginning
  virtual void init();

  /// operator* delivers a reference to data accessor (current chunk)
  /// @return a reference to the current chunk
  virtual const IConstDataAccessor& operator*() const;
	
  /// Checks whether there are more data available.
  /// @return True if there are more data available
  virtual casa::Bool hasMore() const throw();
	
  /// advance the iterator one step further 
  /// @return True if there are more data (so constructions like 
  ///         while(it.next()) {} are possible)
  virtual casa::Bool next();
private:
  casa::Table itsMS;
  boost::shared_ptr<ITableDataSelectorImpl const>  itsSelector;
  boost::shared_ptr<IDataConverterImpl const>  itsConverter;
  /// the maximum allowed number of rows in the accessor.
  casa::uInt itsMaxChunkSize;
  casa::TableIterator itsTabIterator;
  casa::uInt itsCurrentTopRow;
  /// for now use the stub, although it won't provide read on
  /// demand capability
  mutable DataAccessorStub itsAccessor;
};


} // end of namespace synthesis

} // end of namespace conrad
#endif // #ifndef TABLE_CONST_DATA_ITERATOR_H
