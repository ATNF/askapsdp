/// @file 
/// @brief An extention of ITableHolder to derived information
/// @details A class given in this file is closely related to
/// SubtableInfoHolder. This interface provides a single
/// pure virtual function, which returns a const reference to
/// SubtableInfoHolder. An additional level of wrapping is required to
/// store the class actually holding the table & its derived
/// data by pointer (managed by a smart pointer template).
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

#ifndef I_TABLE_INFO_ACCESSOR_H
#define I_TABLE_INFO_ACCESSOR_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/ITableHolder.h>
#include <dataaccess/ISubtableInfoHolder.h>
#include <dataaccess/ITableManager.h>

namespace askap {

namespace synthesis {

/// @brief An extention of ITableHolder to derived information
/// @details A class given in this file is closely related to
/// SubtableInfoHolder. This interface provides a single
/// pure virtual function, which returns a const reference to
/// SubtableInfoHolder. An additional level of wrapping is required to
/// store the class actually holding the table & its derived
/// data by pointer (managed by a smart pointer template).
/// @ingroup dataaccess_tm
struct ITableInfoAccessor : virtual public ITableHolder {
  /// @return a reference to ISubtableInfoHolder
  virtual const ISubtableInfoHolder& subtableInfo() const = 0;
  
  /// @return a shared pointer on infoHolder
  virtual const boost::shared_ptr<ITableManager const>&
                        getTableManager() const throw() = 0;

  /// @return a reference to IMiscTableInfoHolder
  virtual const IMiscTableInfoHolder& miscTableInfo() const = 0;

};

} // namespace synthesis

} // namespace askap

#endif // #define I_TABLE_INFO_ACCESSOR_H
