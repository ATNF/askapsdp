/// @file ITableDataSelectorImpl.h
///
/// ITableDataSelectorImpl: Interface for data selection to be used within
///                the table-based implementation of the layer. The
///                end user interacts with the IDataSelector interface 
///                only.
///
///                If (or when) we have different data sources 
///                table-independent functionality can be split out into a
///                separate interface (i.e. IDataSelectorImpl), which could
///                be a base class for this one.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_TABLE_DATA_SELECTOR_IMPL_H
#define I_TABLE_DATA_SELECTOR_IMPL_H

/// boost includes
#include <boost/shared_ptr.hpp>

/// casa includes
#include <tables/Tables/ExprNode.h>

/// own includes
#include <dataaccess/IDataConverterImpl.h>
#include <dataaccess/IDataSelector.h>

namespace conrad {

namespace synthesis {

class ITableDataSelectorImpl : virtual public IDataSelector
{
public:
  /// Obtain a table expression node for selection. This method is
  /// used in the implementation of the iterator to form a subtable
  /// obeying the selection criteria specified by the user via
  /// IDataSelector interface
  ///
  /// @param conv  a shared pointer to the converter, which is used to sort
  ///              out epochs and other measures used in the selection
  virtual const casa::TableExprNode& getTableSelector(const
               boost::shared_ptr<IDataConverterImpl const> &conv) const = 0;
};
  
} // namespace synthesis
  
} // namespace conrad
  
#endif // #ifndef I_TABLE_DATA_SELECTOR_IMPL_H
