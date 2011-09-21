/// @file
/// @brief an adapter to both IConstDataIterator and IDataIterator
///
/// @details This class is similar to DataAccessorAdapter, but it adaps
/// the iterator interface (rather than accessor interface). The current
/// design of the synthesis code is largely iterator-based. This adapter
/// and derived classes allow to reduce ugliness of the design in the case
/// when break of the iteration is required. For example, one of the motivations
/// is to provide more than one calibration solution per dataset (i.e. per iterator).
/// In its current form, this iterator could be used if one needs to adapt a 
/// const iterator when non-const iterator is required by the interface
/// (yes, the current code is quite messy), but all operations are read-only.
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

#ifndef DATA_ITERATOR_ADAPTER_H
#define DATA_ITERATOR_ADAPTER_H

// own includes
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataIterator.h>
#include <utils/ChangeMonitor.h>

// boost includes
#include <boost/shared_ptr.hpp>

namespace askap {

namespace accessors {

/// @details This class is similar to DataAccessorAdapter, but it adaps
/// the iterator interface (rather than accessor interface). The current
/// design of the synthesis code is largely iterator-based. This adapter
/// and derived classes allow to reduce ugliness of the design in the case
/// when break of the iteration is required. For example, one of the motivations
/// is to provide more than one calibration solution per dataset (i.e. per iterator).
/// In its current form, this iterator could be used if one needs to adapt a 
/// const iterator when non-const iterator is required by the interface
/// (yes, the current code is quite messy), but all operations are read-only.
/// @ingroup dataaccess_hlp
class DataIteratorAdapter : virtual public IDataIterator
{
public:
  // constructors
  
  /// @brief default constructor to get an uninitialised adapter
  DataIteratorAdapter();
  
  /// @brief setup with the given iterator
  /// @details
  /// @param[in] iter shared pointer to iterator to be wrapped
  /// @note the code tries to cast the shared pointer to a non-const iterator type. If
  /// successul, non-const methods of the adapter will also work
  explicit DataIteratorAdapter(const boost::shared_ptr<IConstDataIterator> &iter);
  
  // methods to associate, detach and check status of this accessor
  
  /// @brief associate this adapter
  /// @details This method associates the adapter with the given iterator
  /// @param[in] iter shared pointer to iterator to be wrapped
  /// @note the code tries to cast the shared pointer to a non-const iterator type. If
  /// successul, non-const methods of the adapter will also work
  void associate(const boost::shared_ptr<IConstDataIterator> &iter);
  
  /// @brief check whether the adapter is associated with some iterator
  /// @return true if adapter is associated with an iterator, false otherwise
  bool isAssociated() const;
  
  /// @brief detach adapter from the currently associated iterator, if any
  void detach();
  
  /// @brief check whether write operation is permitted
  /// @return true, if write is possible
  bool canWrite() const;
  
protected:
  /// @brief obtain change monitor
  /// @details It can be used in derived classes to compare whether we still
  /// deal with the same iterator as the one which might be used for some more evolved calculations.
  /// This change monitor tracks detach and associate calls and allows to avoid 
  /// overriding of all these methods (and to avoid making them virtual), if a simple
  /// caching of derived products is found to be necessary in the derived classes.
  /// A comparison of two change monitors with a non-equal result means that the
  /// accessor was updated some time in between these two calls
  inline scimath::ChangeMonitor changeMonitor() const { return itsChangeMonitor; }
  
private:
  /// @brief shared pointer to const iterator
  /// @details It is always initialised, if the adapter is attached to some iterator
  boost::shared_ptr<IConstDataIterator> itsConstIterator;
  
  /// @brief shared pointer to non-const iterator
  /// @details This data field is initialised only if the adapter is attached to some
  /// non-const iterator. It points to the same object as itsConstIterator and used
  /// in non-const methods.
  boost::shared_ptr<IDataIterator> itsIterator;
  
  /// @brief change monitor for iterators
  /// @details we may need to know when iterator is
  /// updated in the derived classes. Change monitor provides
  /// an efficient way of doing it.
  scimath::ChangeMonitor itsChangeMonitor;  
};

} // namespace accessors

} // namespace askap

#endif // #ifndef DATA_ITERATOR_ADAPTER_H

