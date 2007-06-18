/// @file TableConstDataIterator.h
///
/// @brief Implementation of IConstDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// This is an implementation in the table-based case.
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
#include <dataaccess/TableConstDataAccessor.h>

namespace conrad {

namespace synthesis {

/// @brief Implementation of IConstDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// This is an implementation in the table-based case.
class TableConstDataIterator : public IConstDataIterator
{
public:
  /// @param[in] ms the measurement set to use (as a reference to table)
  /// @param[in] sel shared pointer to selector
  /// @param[in] conv shared pointer to converter
  /// @param[in] maxChunkSize maximum number of rows per accessor
  TableConstDataIterator(const casa::Table &ms,
              const boost::shared_ptr<ITableDataSelectorImpl const> &sel,
	      const boost::shared_ptr<IDataConverterImpl const> &conv,
	      casa::uInt maxChunkSize = INT_MAX);

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

  /// methods used in the accessor.

  /// @return number of rows in the current accessor
  casa::uInt inline nRow() const throw() { return itsNumberOfRows;}

  /// temporary - access to number of channels and polarizations
  /// it will be determined by the selector, get them from the table
  /// for now

  /// @return number of channels in the current accessor
  casa::uInt inline nChannel() const throw() { return itsNumberOfChannels;}

  /// @return number of channels in the current accessor
  casa::uInt inline nPol() const throw() { return itsNumberOfPols;}

  /// populate the buffer of visibilities with the values of current
  /// iteration
  /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the complex visibility data
  void fillVisibility(casa::Cube<casa::Complex> &vis) const;

  /// populate the buffer with uvw
  /// @param[in] uvw a reference to vector of rigid vectors (3 elemets,
  ///            u,v and w for each row) to fill
  void fillUVW(casa::Vector<casa::RigidVector<casa::Double, 3> >&uvw) const;
  
protected:
  /// setup accessor for a new iteration
  void setUpIteration();
private:
  casa::Table itsMS;
  boost::shared_ptr<ITableDataSelectorImpl const>  itsSelector;
  boost::shared_ptr<IDataConverterImpl const>  itsConverter;
  /// the maximum allowed number of rows in the accessor.
  casa::uInt itsMaxChunkSize;
  casa::TableIterator itsTabIterator;
  /// current group of data returned by itsTabIterator
  casa::Table itsCurrentIteration;
  /// current row in the itsCurrentIteration projected to the row 0
  /// of the data accessor
  casa::uInt itsCurrentTopRow;
  /// number of rows in the current chunk
  casa::uInt itsNumberOfRows;
  /// next two data members are temporary here
  /// we need to use properties of selector when it's ready
  casa::uInt itsNumberOfChannels;
  casa::uInt itsNumberOfPols;
  
  /// for now use the stub, although it won't provide read on
  /// demand capability
  mutable TableConstDataAccessor itsAccessor;
};


} // end of namespace synthesis

} // end of namespace conrad
#endif // #ifndef TABLE_CONST_DATA_ITERATOR_H
