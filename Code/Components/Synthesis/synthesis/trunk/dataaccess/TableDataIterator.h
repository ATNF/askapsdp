/// @file
///
/// @brief Implementation of IDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// TableDataIterator extends the interface further to read-write operations.
/// Each iteration step is represented by the IDataAccessor interface in this
/// case. 
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_DATA_ITERATOR_H
#define TABLE_DATA_ITERATOR_H

// std includes
#include <string>
#include <map>

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/TableConstDataIterator.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/TableInfoAccessor.h>
#include <dataaccess/IDataAccessor.h>
#include <dataaccess/TableBufferDataAccessor.h>


namespace conrad {

namespace synthesis {

/// @brief Implementation of IDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// TableDataIterator extends the interface further to read-write operations.
/// Each iteration step is represented by the IDataAccessor interface in this
/// case. 
class TableDataIterator : public TableConstDataIterator,
                          virtual public IDataIterator,
			  virtual protected TableInfoAccessor
{
public:
  /// @param[in] msManager a manager of the measurement set to use
  /// @param[in] sel shared pointer to selector
  /// @param[in] conv shared pointer to converter
  /// @param[in] maxChunkSize maximum number of rows per accessor
  TableDataIterator(const boost::shared_ptr<ITableManager const>
              &msManager,
              const boost::shared_ptr<ITableDataSelectorImpl const> &sel,
	      const boost::shared_ptr<IDataConverterImpl const> &conv,
	      casa::uInt maxChunkSize = INT_MAX);

  /// destructor required to sync buffers on the last iteration
  virtual ~TableDataIterator();

  /// @brief operator* delivers a reference to data accessor (current chunk)
  /// @details
  /// @return a reference to the current chunk
  /// @note
  /// constness of the return type is changed to allow read/write
  /// operations.
  ///
  virtual IDataAccessor& operator*() const;

  /// @brief Switch the output of operator* and operator-> to one of 
  /// the buffers.
  /// @details This is meant to be done to provide the same 
  /// interface for a buffer access as exists for the original 
  /// visibilities (e.g. it->visibility() to get the cube).
  /// It can be used for an easy substitution of the original 
  /// visibilities to ones stored in a buffer, when the iterator is
  /// passed as a parameter to mathematical algorithms.   
  /// The operator* and operator-> will refer to the chosen buffer
  /// until a new buffer is selected or the chooseOriginal() method
  /// is executed to revert operators to their default meaning
  /// (to refer to the primary visibility data).  
  /// @param[in] bufferID  the name of the buffer to choose
  ///
  virtual void chooseBuffer(const std::string &bufferID);
  
  /// Switch the output of operator* and operator-> to the original
  /// state (present after the iterator is just constructed) 
  /// where they point to the primary visibility data. This method
  /// is indended to cancel the results of chooseBuffer(std::string)
  ///
  virtual void chooseOriginal();
  
  /// @brief obtain any associated buffer for read/write access.
  /// @details The buffer is identified by its bufferID. The method 
  /// ignores a chooseBuffer/chooseOriginal setting.  
  /// @param[in] bufferID the name of the buffer requested
  /// @return a reference to writable data accessor to the
  ///         buffer requested
  virtual IDataAccessor& buffer(const std::string &bufferID) const;    

  /// Restart the iteration from the beginning
  void init();

  /// @brief Checks whether there are more data available.
  /// @details
  /// @return True if there are more data available
  /// @note this method can be removed from this class
  /// when we migrate to g++-4.1 and inheritance of
  /// interfaces is made virtual.
  virtual casa::Bool hasMore() const throw();
	
  /// advance the iterator one step further 
  /// @return True if there are more data (so constructions like 
  ///         while(it.next()) {} are possible)
  virtual casa::Bool next();

  // to make it public instead of protected
  using TableConstDataIterator::getAccessor;

  /// populate the cube with the data stored in the given buffer  
  /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the complex visibility data
  /// @param[in] name a name of the buffer to work with
  virtual void readBuffer(casa::Cube<casa::Complex> &vis,
                          const std::string &name) const;

  /// write the cube back to the given buffer  
  /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
  ///            cube to fill with the complex visibility data
  /// @param[in] name a name of the buffer to work with
  virtual void writeBuffer(const casa::Cube<casa::Complex> &vis,
                           const std::string &name) const;
  			  

private:
  /// shared pointer to the data accessor associated with either an active
  /// buffer or original visibilites. The actual type held by the pointer
  /// may vary.
  boost::shared_ptr<IDataAccessor> itsActiveBufferPtr;

  /// a container of buffers
  mutable std::map<std::string,
           boost::shared_ptr<TableBufferDataAccessor> >  itsBuffers;

  /// shared pointer to data accessor associated with original visibilities
  /// (initialized at the constructor)
  boost::shared_ptr<IDataAccessor> itsOriginalVisAccessor;

  /// counter of the iteration steps. It is used to store the buffers
  /// to the appropriate cell of the disk table
  casa::uInt itsIterationCounter;
};

} // end of namespace synthesis

} // end of namespace conrad


#endif  // #ifndef TABLE_DATA_ITERATOR_H
