/// @file 
/// @brief An extention of ITableHolder to derived information
/// @details An implementation of this interface constructs holders of
/// derived information (extracted from subtables) on demand. This
/// interface is intended to provide  access to this data stored in the
/// subtable via abstract classes of individual  holders. Examples of
/// derived information include:
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

#ifndef I_SUBTABLE_INFO_HOLDER_H
#define I_SUBTABLE_INFO_HOLDER_H

#include <dataaccess/ITableHolder.h>
#include <dataaccess/ITableDataDescHolder.h>

namespace conrad {

namespace synthesis {

/// @brief An extention of ITableHolder to derived information
/// @details An implementation of this interface constructs holders of
/// derived information (extracted from subtables) on demand. This
/// interface is intended to provide  access to this data stored in the
/// subtable via abstract classes of individual  holders. Examples of
/// derived information include:
///     1. feed information,
///     2. data description indices,
///     3. spectral window ids.
/// Such design allows to avoid parsing of all possible subtables and
/// building all possible derived information (which can be time consuming)
/// when the measurement set is opened.
struct ISubtableInfoHolder : virtual public ITableHolder {

   /// @return a reference to the handler of the DATA_DESCRIPTION subtable
   virtual const ITableDataDescHolder& getDataDescription() const = 0;
  
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_SUBTABLE_INFO_HOLDER_H
