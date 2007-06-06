/// @file TableMeasureFieldSelector.h
///
/// TableMeasureFieldSelector: partial implementation of an interface to
///                     constrain a table selection
///                     object (expression node) for a field which is
///                     a measure (i.e. requires a fully defined converter
///                     to complete processing)
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_MEASURE_FIELD_SELECTOR_H
#define TABLE_MEASURE_FIELD_SELECTOR_H

/// own includes
#include <dataaccess/ITableMeasureFieldSelector.h>

namespace conrad {

namespace synthesis {

class TableMeasureFieldSelector : ITableMeasureFieldSelector {
public:
   /// set the converter to use. It should be fully specified somewhere
   /// else before the actual selection can take place. This method
   /// just stores a shared pointer on the converter for future use.
   /// It doesn't require all frame information to be set, etc.
   ///
   /// @param conv shared pointer to the converter object to use
   ///
   virtual void setConverter(const boost::shared_ptr<IDataConverter> &conv)
                             throw();
protected:
   /// obtain a converter object to use
   /// @return a const reference to the converter object associated with
   ///         this class
   const IDataConverter& converter() const throw();
private:
   /// converter object
   boost::shared_ptr<IDataConverter> itsConverter;
};


} // namespace conrad

} // namespace conrad

#endif // #ifndef TABLE_MEASURE_FIELD_SELECTOR_H

