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

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>

// own includes
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataConverterImpl.h>
#include <dataaccess/ITableDataSelectorImpl.h>
#include <dataaccess/TableConstDataAccessor.h>
#include <dataaccess/TableInfoAccessor.h>
#include <dataaccess/ITableManager.h>

namespace conrad {

namespace synthesis {

/// @brief Implementation of IConstDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// This is an implementation in the table-based case.
/// @ingroup dataaccess_tab
class TableConstDataIterator : virtual public IConstDataIterator,
                               virtual protected TableInfoAccessor
{
public:
  /// @brief constructor of the const iterator
  /// @param[in] msManager a manager of the measurement set to use
  /// @param[in] sel shared pointer to selector
  /// @param[in] conv shared pointer to converter
  /// @param[in] maxChunkSize maximum number of rows per accessor
  TableConstDataIterator(const boost::shared_ptr<ITableManager const>
              &msManager,
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

  /// populate the buffer with frequencies
  /// @param[in] freq a reference to a vector to fill
  void fillFrequency(casa::Vector<casa::Double> &freq) const;

  /// @return the time stamp  
  casa::Double getTime() const;

  /// populate the buffer with IDs of the first antenna
  /// @param[in] ids a reference to a vector to fill
  void fillAntenna1(casa::Vector<casa::uInt> &ids) const;

  /// populate the buffer with IDs of the second antenna
  /// @param[in] ids a reference to a vector to fill
  void fillAntenna2(casa::Vector<casa::uInt> &ids) const;

  /// populate the buffer with IDs of the first feed
  /// @param[in] ids a reference to a vector to fill
  void fillFeed1(casa::Vector<casa::uInt> &ids) const;

  /// populate the buffer with IDs of the second feed
  /// @param[in] ids a reference to a vector to fill
  void fillFeed2(casa::Vector<casa::uInt> &ids) const;

protected:
  /// @brief a helper method to read a column with IDs of some sort
  /// @details It reads the column of casa::Int and fills a Vector of
  /// casa::uInt. A check to ensure all numbers are non-negative is done
  /// in the debug mode.
  /// @param[in] ids a reference to a vector to fill
  /// @param[in] name a name of the column to read
  void fillVectorOfIDs(casa::Vector<casa::uInt> &ids,
                       const casa::String &name) const;

  /// setup accessor for a new iteration
  void setUpIteration();

  /// @brief method ensures that the chunk has uniform DATA_DESC_ID
  /// @details This method reduces itsNumberOfRows to the achieve
  /// uniform DATA_DESC_ID reading for all rows in the current chunk.
  /// The resulting itsNumberOfRows will be 1 or more.
  /// itsAccessor's spectral axis cache is reset if new DATA_DESC_ID is
  /// different from itsCurrentDataDescID
  /// This method also sets up itsNumberOfPols and itsNumberOfChannels
  /// when DATA_DESC_ID changes (and therefore at the first run as well)
  void makeUniformDataDescID();

  /// obtain a reference to the accessor (for derived classes)
  inline const TableConstDataAccessor& getAccessor() const throw()
  { return itsAccessor;}

private:
  /// accessor (a chunk of data) 
  /// although the accessor type can be different
  TableConstDataAccessor itsAccessor;  

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

  /// current DATA_DESC_ID, the iteration is broken if this
  /// ID changes
  casa::Int itsCurrentDataDescID;
};


} // end of namespace synthesis

} // end of namespace conrad
#endif // #ifndef TABLE_CONST_DATA_ITERATOR_H
