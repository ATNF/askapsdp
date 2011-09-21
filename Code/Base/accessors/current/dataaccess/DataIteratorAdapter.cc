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


} // namespace accessors

} // namespace askap

