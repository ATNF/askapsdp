/// @file
/// @brief an adapter to both IConstDataAccessor and IDataAccessor
///
/// @details This class is very similar to MetaDataAccessor. It essentially
/// implements the alternative approach mentioned in the documentation for
/// MetaDataAccessor, i.e. the original accessor is held by the shared pointer.
/// In principle, we could've used MetaDataAccessor instead of this class (or
/// convert all code using MetaDataAccessor to use this class). But in some
/// applications holding the original accessor by a reference leads to an
/// ugly design.
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/DataIteratorAdapter.h>

namespace askap {

namespace accessors {

/// @brief default constructor to get an uninitialised adapter
DataIteratorAdapter::DataIteratorAdapter() {}
  
/// @brief setup with the given iterator
/// @details
/// @param[in] iter shared pointer to iterator to be wrapped
/// @note the code tries to cast the shared pointer to a non-const iterator type. If
/// successul, non-const methods of the adapter will also work
DataIteratorAdapter::DataIteratorAdapter(const boost::shared_ptr<IConstDataIterator> &iter) : itsConstIterator(iter),
     itsIterator(boost::dynamic_pointer_cast<IDataIterator>(iter)) {}
  
// methods to associate, detach and check status of this accessor
  
/// @brief associate this adapter
/// @details This method associates the adapter with the given iterator
/// @param[in] iter shared pointer to iterator to be wrapped
/// @note the code tries to cast the shared pointer to a non-const iterator type. If
/// successul, non-const methods of the adapter will also work
void DataIteratorAdapter::associate(const boost::shared_ptr<IConstDataIterator> &iter) {
   itsConstIterator = iter;
   itsIterator = boost::dynamic_pointer_cast<IDataIterator>(iter);   
   itsChangeMonitor.notifyOfChanges();
}
  
/// @brief check whether the adapter is associated with some iterator
/// @return true if adapter is associated with an iterator, false otherwise
bool DataIteratorAdapter::isAssociated() const
{
  return itsConstIterator;
}
  
/// @brief detach adapter from the currently associated iterator, if any
void DataIteratorAdapter::detach()
{
  itsConstIterator.reset();
  itsIterator.reset();
  itsChangeMonitor.notifyOfChanges();
}
  
/// @brief check whether write operation is permitted
/// @return true, if write is possible
bool DataIteratorAdapter::canWrite() const
{
  return itsIterator;
}

/// @brief obtain a reference to associated iterator for read-only access
/// @details This method checks the validity of the shared pointer and
/// returns a reference of the const iterator type. The operation should always be
/// successful, provided this adapter is associated with an iterator. Otherwise an
/// exception is thrown
/// @return a refernce to associated read-only iterator
IConstDataIterator & DataIteratorAdapter::roIterator() const
{
  ASKAPCHECK(itsConstIterator, "DataIteratorAdapter is not associated with any iterator");
  return *itsConstIterator;
}

/// @brief obtain a reference to associated iterator for read-write access
/// @details This method checks that the iterator is writeable (i.e. the appropriate 
/// shared pointer is valid) and returns the reference. An exception
/// is thrown if the associated iterator is of the const type.
/// @return a refernce to associated non-const iterator
IDataIterator & DataIteratorAdapter::rwIterator() const
{
  ASKAPCHECK(isAssociated(), "DataIteratorAdapter is not associated with any iterator");
  ASKAPCHECK(itsIterator, "DataIteratorAdapter is associated with a const iterator, no write possible.");
  return *itsIterator;
}

/// Restart the iteration from the beginning
void DataIteratorAdapter::init() {
  roIterator().init();
  itsAccessorAdapter.detach();
}

/// operator* delivers a reference to data accessor (current chunk)
/// @return a reference to the current chunk
IDataAccessor& DataIteratorAdapter::operator*() const {
  if (canWrite()) {
      itsAccessorAdapter.detach();
      return rwIterator().operator*();
  }
  itsAccessorAdapter.associate(roIterator().operator*());
  return itsAccessorAdapter;
}

/// Checks whether there are more data available.
/// @return True if there are more data available
casa::Bool DataIteratorAdapter::hasMore() const throw() {
  return roIterator().hasMore();
}

/// advance the iterator one step further 
/// @return True if there are more data (so constructions like 
///         while(it.next()) {} are possible)
casa::Bool DataIteratorAdapter::next() {
  itsAccessorAdapter.detach();
  return roIterator().next();
}

/// Switch the output of operator* and operator-> to one of 
/// the buffers. This is meant to be done to provide the same 
/// interface for a buffer access as exists for the original 
/// visibilities (e.g. it->visibility() to get the cube).
/// It can be used for an easy substitution of the original 
/// visibilities to ones stored in a buffer, when the iterator is
/// passed as a parameter to mathematical algorithms. 
/// 
/// The operator* and operator-> will refer to the chosen buffer
/// until a new buffer is selected or the chooseOriginal() method
/// is executed to revert operators to their default meaning
/// (to refer to the primary visibility data).
///
/// @param[in] bufferID  the name of the buffer to choose
///
void DataIteratorAdapter::chooseBuffer(const std::string &bufferID) 
{
  itsAccessorAdapter.detach();
  rwIterator().chooseBuffer(bufferID);
}

/// Switch the output of operator* and operator-> to the original
/// state (present after the iterator is just constructed) 
/// where they point to the primary visibility data. This method
/// is indended to cancel the results of chooseBuffer(casa::uInt)
///
void DataIteratorAdapter::chooseOriginal()
{
  itsAccessorAdapter.detach();
  rwIterator().chooseOriginal();
}

/// return any associated buffer for read/write access. The 
/// buffer is identified by its bufferID. The method 
/// ignores a chooseBuffer/chooseOriginal setting.
/// 
/// @param[in] bufferID the name of the buffer requested
/// @return a reference to writable data accessor to the
///         buffer requested
///
/// Because IDataAccessor has both const and non-const visibility()
/// methods defined separately, it is possible to detect when a
/// write operation took place and implement a delayed writing
IDataAccessor& DataIteratorAdapter::buffer(const std::string &bufferID) const
{
  return rwIterator().buffer(bufferID);
}


} // namespace accessors

} // namespace askap

