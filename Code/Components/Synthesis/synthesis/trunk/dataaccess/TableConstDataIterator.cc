/// @file TableConstDataIterator.cc
///
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// This is implementation in the table-based case.
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own includes
#include <dataaccess/TableConstDataIterator.h>

using namespace conrad;
using namespace synthesis;

/// @param[in] ms the measurement set to use (as a reference to table)
/// @param[in] sel shared pointer to selector
/// @param[in] conv shared pointer to converter
/// @param[in] maxChunkSize maximum number of rows per accessor
TableConstDataIterator::TableConstDataIterator(const casa::Table &ms,
            const boost::shared_ptr<ITableDataSelectorImpl const> &sel,
            const boost::shared_ptr<IDataConverterImpl const> &conv,
	    casa::uInt maxChunkSize) :
	   itsMS(ms), itsSelector(sel), itsConverter(conv),
	   itsMaxChunkSize(maxChunkSize)
{
  init();
}

/// Restart the iteration from the beginning
void TableConstDataIterator::init()
{
  itsCurrentTopRow=0;
  itsTabIterator=casa::TableIterator(itsMS(itsSelector->
                               getTableSelector(itsConverter)),"TIME",
	   casa::TableIterator::DontCare,casa::TableIterator::NoSort);
}

/// operator* delivers a reference to data accessor (current chunk)
/// @return a reference to the current chunk
const IConstDataAccessor& TableConstDataIterator::operator*() const
{
  return itsAccessor;
}
      
/// Checks whether there are more data available.
/// @return True if there are more data available
casa::Bool TableConstDataIterator::hasMore() const throw()
{
  return false;
}
      
/// advance the iterator one step further 
/// @return True if there are more data (so constructions like 
///         while(it.next()) {} are possible)
casa::Bool TableConstDataIterator::next()
{
  return hasMore();
}
