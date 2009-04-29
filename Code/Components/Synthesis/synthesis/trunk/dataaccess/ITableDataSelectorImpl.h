/// @file ITableDataSelectorImpl.h
/// @brief An interface for table data selection
/// @details ITableDataSelectorImpl is an interface for data selection
/// to be used within the table-based implementation of the layer. The
/// end user interacts with the IDataSelector interface only.
///
/// If (or when) we have different data sources 
/// table-independent functionality can be split out into a
/// separate interface (i.e. IDataSelectorImpl), which could
/// be a base class for this one.
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
#ifndef I_TABLE_DATA_SELECTOR_IMPL_H
#define I_TABLE_DATA_SELECTOR_IMPL_H

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes
#include <tables/Tables/ExprNode.h>

// own includes
#include <dataaccess/IDataConverterImpl.h>
#include <dataaccess/IDataSelector.h>

// std includes
#include <string>
#include <utility>

namespace askap {

namespace synthesis {

/// @brief An interface for subtable selection (forms an expression node)
/// @details ITableDataSelectorImpl is an interface for data selection
/// to be used within the table-based implementation of the layer. The
/// end user interacts with the IDataSelector interface only.
///
/// @todo If (or when) we have different data sources 
/// table-independent functionality can be split out into a
/// separate interface (i.e. IDataSelectorImpl), which could
/// be a base class for this one.
/// @ingroup dataaccess_tab
class ITableDataSelectorImpl : virtual public IDataSelector
{
public:
  /// @brief Obtain a table expression node for selection. 
  /// @details This method is
  /// used in the implementation of the iterator to form a subtable
  /// obeying the selection criteria specified by the user via
  /// IDataSelector interface
  ///
  /// @param conv  a shared pointer to the converter, which is used to sort
  ///              out epochs and other measures used in the selection
  /// @return a const reference to table expression node object
  virtual const casa::TableExprNode& getTableSelector(const
               boost::shared_ptr<IDataConverterImpl const> &conv) const = 0;
  
  /// @brief choose data column
  /// @details This method allows to choose any table column as the visibility
  /// data column (e.g. DATA, CORRECTED_DATA, etc). Because this is a
  /// table-specific operation, this method is defined in a table-specific
  /// selector interface and is not present in IDataSelector (therefore,
  /// a dynamic_pointer_cast is likely required).
  /// @param[in] dataColumn column name, which contains visibility data 
  virtual void chooseDataColumn(const std::string &dataColumn) = 0;  
  
  /// @brief clone a selector
  /// @details The same selector can be used to create a number of iterators.
  /// Selector stores a name of the data column to use and, therefore, it can
  /// be changed after some iterators are created. To avoid bugs due to this
  /// reference semantics, the iterator will clone selector in its constructor.
  /// @note This functionality is not exposed to the end user, which
  /// normally interacts with the IDataSelector class only. This is because
  /// cloning is done at the low level (e.g. inside the iterator)
  virtual boost::shared_ptr<ITableDataSelectorImpl const> clone() const = 0;
  
  /// @brief obtain the name of data column
  /// @details This method returns the current name of the data column.
  /// Exact handling is determined in derived classes
  /// @return the name of the data column
  virtual const std::string& getDataColumnName() const throw() = 0;
  
  /// @brief check whether channel selection has been done
  /// @details By default all channels are selected. However, if chooseChannels 
  /// has been called, less channels are returned. This method returns true if
  /// this is the case and false otherwise.
  /// @return true, if a subset of channels has been selected
  virtual bool channelsSelected() const throw() = 0;
  
  /// @brief obtain channel selection
  /// @details By default all channels are selected. However, if chooseChannels 
  /// has been called, less channels are returned by the accessor. This method
  /// returns the number of channels and the first channel (in the full sample) 
  /// selected. If the first element of the pair is negative, no channel-based
  /// selection has been done. This is also checked by channelsSelected method, 
  /// which is probably a prefered way to do this check to retain the code clarity.
  /// @return a pair, the first element gives the number of channels selected and
  /// the second element gives the start channel (0-based)
  virtual std::pair<int,int> getChannelSelection() const throw() = 0;
  
};
  
} // namespace synthesis
  
} // namespace askap
  
#endif // #ifndef I_TABLE_DATA_SELECTOR_IMPL_H
