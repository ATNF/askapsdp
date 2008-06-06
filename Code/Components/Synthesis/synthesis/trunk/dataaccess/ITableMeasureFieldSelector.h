/// @file ITableMeasureFieldSelector.h
/// @brief Interface constraining an expression node for measure fields
/// @details The units and reference frame have to be specified via a
/// fully defined converter to form an expression node selecting a subtable
/// based on some measure-type field (e.g. time range). This interface
/// provide appropriate methods.
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

#ifndef I_TABLE_MEASURE_FIELD_SELECTOR_H
#define I_TABLE_MEASURE_FIELD_SELECTOR_H

// casa includes
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/IDataConverterImpl.h>

// boost includes
#include <boost/shared_ptr.hpp>

namespace askap {

namespace synthesis {

/// @brief Interface constraining an expression node for measure fields
/// @details The units and reference frame have to be specified via a
/// fully defined converter to form an expression node selecting a subtable
/// based on some measure-type field (e.g. time range). This interface
/// provide appropriate methods.
/// @ingroup dataaccess_tab
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

} // namespace askap

} // namespace askap

#endif // #ifndef I_TABLE_MEASURE_FIELD_SELECTOR_H
