/// @file 
/// @brief Implementation of ISubtableInfoHolder
/// @details This class manages and constructs handlers of
/// derived information (extracted from subtables) on demand.
/// The access to this information is via abstract classes of
/// individual  holders. Examples of derived information include:
///     1. feed information,
///     2. data description indices,
///     3. spectral window ids.
/// Such design allows to avoid parsing of all possible subtables and
/// building all possible derived information (which can be time consuming)
/// when the measurement set is opened.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef SUBTABLE_INFO_HOLDER_H
#define SUBTABLE_INFO_HOLDER_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/ISubtableInfoHolder.h>
#include <dataaccess/ITableDataDescHolder.h>

namespace conrad {

namespace synthesis {

/// @brief Implementation of ISubtableInfoHolder
/// @details This class manages and constructs handlers of
/// derived information (extracted from subtables) on demand.
/// The access to this information is via abstract classes of
/// individual  holders. Examples of derived information include:
///     1. feed information,
///     2. data description indices,
///     3. spectral window ids.
/// Such design allows to avoid parsing of all possible subtables and
/// building all possible derived information (which can be time consuming)
/// when the measurement set is opened.
struct SubtableInfoHolder : virtual public ISubtableInfoHolder {

   /// @brief obtain data description holder
   /// @details A MemTableDataDescHolder is constructed on the first call
   /// to this method and a reference to it is always returned later
   /// @return a reference to the handler of the DATA_DESCRIPTION subtable
   virtual const ITableDataDescHolder& getDataDescription() const;

protected:   
   /// initialize itsDataDescHolder with an instance of MemTableDataDescHolder.
   virtual void initDataDescHolder() const;
private:
   /// smart pointer to data description holder
   mutable boost::shared_ptr<ITableDataDescHolder const> itsDataDescHolder;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef SUBTABLE_INFO_HOLDER_H
