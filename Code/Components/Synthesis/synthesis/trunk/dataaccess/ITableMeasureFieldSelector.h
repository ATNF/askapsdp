/// @file ITableMeasureFieldSelector.h
///
/// ITableMeasureFieldSelector: an interface to constrain a table selection
///                     object (expression node) for a field which is
///                     a measure (i.e. requires a fully defined converter
///                     to complete processing)
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_MEASURE_FIELD_SELECTOR_H
#define I_TABLE_MEASURE_FIELD_SELECTOR_H

/// casa includes
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/Table.h>

/// own includes
#include <dataaccess/IDataConverter.h>

/// boost includes
#include <boost/shared_ptr.hpp>

namespace conrad {

namespace synthesis {

class ITableMeasureFieldSelector {
public:
   /// to keep the compiler happy
   virtual ~ITableMeasureFieldSelector();

   /// set the converter to use. It should be fully specified somewhere
   /// else before the actual selection can take place. This method
   /// just stores a shared pointer on the converter for future use.
   /// It doesn't require all frame information to be set, etc.
   ///
   /// @param conv shared pointer to the converter object to use
   ///
   virtual void setConverter(const boost::shared_ptr<IDataConverter> &conv)
                             throw() = 0;

   /// main method, updates table expression node to narrow down the selection
   ///
   /// @param tex a reference to table expression to use
   virtual void updateTableExpression(casa::TableExprNode &tex) = 0;
};

} // namespace conrad

} // namespace conrad

#endif // #ifndef I_TABLE_MEASURE_FIELD_SELECTOR_H
