/// @file TableMeasureFieldSelector.cc
/// @brief partial implementation of ITableMeasureFieldSelector (handles converter)
/// @details This is a partial implementation of an interface to
/// constrain a table selection object (expression node)
/// for a field which is a measure, i.e. requires a
/// fully defined converter for processing
/// (base interface is ITableMeasureSelector)
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

/// own includes
#include <dataaccess/TableMeasureFieldSelector.h>

using namespace askap;
using namespace askap::accessors;

/// set the converter to use. It should be fully specified somewhere
/// else before the actual selection can take place. This method
/// just stores a shared pointer on the converter for future use.
/// It doesn't require all frame information to be set, etc.
///
/// @param conv shared pointer to the converter object to use
///
void TableMeasureFieldSelector::setConverter(const
               boost::shared_ptr<IDataConverterImpl const> &conv) throw()
{
  itsConverter=conv;
}

/// obtain a converter object to use
/// @return a const reference to the converter object associated with
///         this class
const IDataConverterImpl& TableMeasureFieldSelector::converter() const throw()
{
  // we may need to put a debug assert here
  return *itsConverter;
}
