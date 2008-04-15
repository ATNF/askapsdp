/// @file TableConstDataIterator.h
///
/// @brief Implementation of IConstDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// This is an implementation in the table-based case.
/// 
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_CONST_DATA_ITERATOR_H
#define TABLE_CONST_DATA_ITERATOR_H

// std includes
#include <string>

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
#include <dataaccess/CachedAccessorField.tcc>

namespace askap {

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
  
  /// @brief read flagging information
  /// @details populate the buffer of flags with the information
  /// read in the current iteration
  /// @param[in] flag a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the flag information (each element has
  ///            bool type)
  void fillFlag(casa::Cube<casa::Bool> &flag) const;

  /// populate the buffer with uvw
  /// @param[in] uvw a reference to vector of rigid vectors (3 elemets,
  ///            u,v and w for each row) to fill
  void fillUVW(casa::Vector<casa::RigidVector<casa::Double, 3> >&uvw) const;

  /// populate the buffer with frequencies
  /// @param[in] freq a reference to a vector to fill
  void fillFrequency(casa::Vector<casa::Double> &freq) const;

  /// @return the time stamp in the table's native frame/units
  /// @note this method doesn't do any caching. It reads the table each
  /// time it is called. It is intended for use from the accessor only, where
  /// caching is done. 
  casa::Double getTime() const;
  
  /// @brief an alternative way to get the time stamp
  /// @details This method uses the accessor to get cached time stamp. It
  /// is returned as an epoch measure.
  casa::MEpoch currentEpoch() const; 
  
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
  
  /// fill the buffer with the pointing directions of the first antenna/feed
  /// @param[in] dirs a reference to a vector to fill
  void fillPointingDir1(casa::Vector<casa::MVDirection> &dirs) const;

  /// fill the buffer with the pointing directions of the second antenna/feed
  /// @param[in] dirs a reference to a vector to fill
  void fillPointingDir2(casa::Vector<casa::MVDirection> &dirs) const;

protected:
  /// @brief read an array column of the table into a cube
  /// @details populate the buffer provided with the information
  /// read in the current iteration. This method is templated and can be
  /// used for both visibility and flag data fillers.
  /// @param[in] cube a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the information from table
  /// @param[in] columnName a name of the column to read
  template<typename T>
  void fillCube(casa::Cube<T> &cube, const std::string &columnName) const;

  /// @brief A helper method to fill a given vector with pointingdirections.
  /// @details fillPointingDir1 and fillPointingDir2 methods do very similar
  /// operations, which differ only by the feedIDs and antennaIDs used.
  /// This method encapsulates these common operations
  /// @param[in] dirs a reference to a vector to fill
  /// @param[in] antIDs a vector with antenna IDs
  /// @param[in] feedIDs a vector with feed IDs
  void fillVectorOfPointings(casa::Vector<casa::MVDirection> &dirs,
               const casa::Vector<casa::uInt> &antIDs,
               const casa::Vector<casa::uInt> &feedIDs) const;
  
  
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

  /// @brief method ensures that the chunk has a uniform DATA_DESC_ID
  /// @details This method reduces itsNumberOfRows to achieve
  /// uniform DATA_DESC_ID reading for all rows in the current chunk.
  /// The resulting itsNumberOfRows will be 1 or more.
  /// itsAccessor's spectral axis cache is reset if new DATA_DESC_ID is
  /// different from itsCurrentDataDescID
  /// This method also sets up itsNumberOfPols and itsNumberOfChannels
  /// when DATA_DESC_ID changes (and therefore at the first run as well)
  void makeUniformDataDescID();

  /// @brief method ensures that the chunk has a uniform FIELD_ID
  /// @details This method reduces itsNumberOfRows until FIELD_ID is
  /// the same for all rows in the current chunk. The resulting 
  /// itsNumberOfRows will be 1 or more. If itsUseFieldID is false,
  /// the method returns without doing anything. itsAccessor's direction
  /// cache is reset if new FIELD_ID is different from itsCurrentFieldID
  /// (and it sets it up at the first run as well)
  void makeUniformFieldID();

  /// obtain a reference to the accessor (for derived classes)
  inline const TableConstDataAccessor& getAccessor() const throw()
  { return itsAccessor;}

  /// @brief Fill internal buffer with the pointing directions.
  /// @details  The layout of this buffer is the same as the layout of
  /// the FEED subtable for current time and spectral window. 
  /// getAntennaIDs and getFeedIDs methods of the 
  /// subtable handler can be used to unwrap this 1D array. 
  /// The buffer can invalidated if the time changes (i.e. for an alt-az array),
  /// for an equatorial array this happends only if the FEED or FIELD subtable
  /// are time-dependent
  /// @param[in] dirs a reference to a vector to fill
  void fillDirectionCache(casa::Vector<casa::MVDirection> &dirs) const;

  /// @brief obtain a current spectral window ID
  /// @details This method obtains a spectral window ID corresponding to the
  /// current data description ID and tests its validity
  /// @return current spectral window ID
  casa::uInt currentSpWindowID() const;  
  
  /// @brief obtain the current iteration of the table iterator
  /// @details This class uses TableIterator behind the scene. This method
  /// returns the current iteration, which can be used in derived classes
  /// (e.g. for read-write access)
  /// @return a const reference to table object representing the current iteration
  inline const casa::Table& getCurrentIteration() const throw() 
       {return itsCurrentIteration;}
  
  /// @brief obtain the current top row
  /// @details This class uses TableIterator behind the scence. One iteration
  /// of the table iterator may cover more than one iteration of the iterator
  /// represented by this class. The result of this method is a row number, 
  /// where current data accessor starts.
  /// @return row number in itsCurrentItrertion corresponding to row 0 of the
  /// accessor at this iteration
  inline casa::uInt getCurrentTopRow() const throw() {return itsCurrentTopRow;}
  
  /// @brief obtain the name of the data column
  /// @details The visibility data can be taken not only from the DATA column,
  /// but from any other appropriate column, e.g. CORRECTED_DATA. This method
  /// returns the name of the column used to store such data. We need it in
  /// derived classes to perform writing
  /// @return name of the table column with visibility data
  const std::string& getDataColumnName() const throw();

  /// @brief obtain a reference direction for the current iteration
  /// @details Currently we assume that the dish pointing centre stays
  /// fixed for the whole chunk. We break the iteration, if necessary
  /// to achieve this. This helper method extracts the reference direction
  /// from the FIELD subtable using either FIELD_ID, or current time if
  /// the former is not supported by the main table.
  /// @return a reference to direction measure
  const casa::MDirection& getCurrentReferenceDir() const;
  
private:
  /// accessor (a chunk of data) 
  /// although the accessor type can be different
  TableConstDataAccessor itsAccessor;  

  boost::shared_ptr<ITableDataSelectorImpl const>  itsSelector;
  boost::shared_ptr<IDataConverterImpl>  itsConverter;
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
  
  /// @brief current FIELD_ID.
  /// @details This ID is tracked if FIELD_ID column is present in the 
  /// table. The iteration is broken if this ID changes.
  casa::Int itsCurrentFieldID;
  
  /// @brief a flag showing that FIELD_ID column should be used.
  /// @details There are two ways to discriminate between different pointings:
  /// use FIELD_ID column, if it is present in the table, and check times. If
  /// this flag is set, the iterator will check FIELD_ID column (an exception is
  /// raised if the column doesn't exist). The constructor checks the presence
  /// of the FIELD_ID column and set this flag if it exists. The flag is introduced
  /// to allow in the future to force the code to use time instead of FIELD_ID, even
  /// if the latter is present.
  bool itsUseFieldID;
  
  /// internal buffer for pointing offsets for the whole current cache
  /// of the Feed subtable handler
  CachedAccessorField<casa::Vector<casa::MVDirection> > itsDirectionCache;  
};


} // end of namespace synthesis

} // end of namespace askap
#endif // #ifndef TABLE_CONST_DATA_ITERATOR_H
