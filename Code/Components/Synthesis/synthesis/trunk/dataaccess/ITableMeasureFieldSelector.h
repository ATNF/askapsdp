/// @file ITableMeasureFieldSelector.h
/// @brief Interface constraining an expression node for measure fields
/// @details The units and reference frame have to be specified via a
/// fully defined converter to form an expression node selecting a subtable
/// based on some measure-type field (e.g. time range). This interface
/// provide appropriate methods.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_MEASURE_FIELD_SELECTOR_H
#define I_TABLE_MEASURE_FIELD_SELECTOR_H

// casa includes
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/IDataConverterImpl.h>

// boost includes
#include <boost/shared_ptr.hpp>

namespace conrad {

namespace synthesis {

/// @brief Interface constraining an expression node for measure fields
/// @details The units and reference frame have to be specified via a
/// fully defined converter to form an expression node selecting a subtable
/// based on some measure-type field (e.g. time range). This interface
/// provide appropriate methods.
/// @ingroup dataaccess
class ITableMeasureFieldSelector {
public:
   /// to keep the compiler happy
   virtual ~ITableMeasureFieldSelector();

   /// set the converter to use. It should be fully specified somewhere
   /// else before the actual selection can take place. This method
   /// just stores a shared pointer on the converter for future use.
   /// It doesn't require all frame information to be set, etc.
   ///
   /// @param[in] conv shared pointer to the converter object to use
   ///
   virtual void setConverter(const
            boost::shared_ptr<IDataConverterImpl const> &conv) throw() = 0;

   /// main method, updates table expression node to narrow down the selection
   ///
   /// @param[in] tex a reference to table expression to use
   virtual void updateTableExpression(casa::TableExprNode &tex) const = 0;
};

} // namespace conrad

} // namespace conrad

#endif // #ifndef I_TABLE_MEASURE_FIELD_SELECTOR_H
