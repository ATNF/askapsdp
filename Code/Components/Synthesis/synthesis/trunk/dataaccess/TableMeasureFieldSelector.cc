/// @file TableMeasureFieldSelector.cc
/// @brief partial implementation of ITableMeasureFieldSelector (handles converter)
/// @details This is a partial implementation of an interface to
/// constrain a table selection object (expression node)
/// for a field which is a measure, i.e. requires a
/// fully defined converter for processing
/// (base interface is ITableMeasureSelector)
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own includes
#include <dataaccess/TableMeasureFieldSelector.h>

using namespace conrad;
using namespace conrad::synthesis;

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
