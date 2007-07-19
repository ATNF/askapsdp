/// @file 
/// @brief A class which holds derived information
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

#include <dataaccess/IHolder.h>
#include <dataaccess/ITableDataDescHolder.h>
#include <dataaccess/ITableSpWindowHolder.h>
#include <dataaccess/IBufferManager.h>
#include <dataaccess/IFeedSubtableHandler.h>
#include <dataaccess/IFieldSubtableHandler.h>
#include <dataaccess/IAntennaSubtableHandler.h>

namespace conrad {

namespace synthesis {

/// @brief A class which holds derived information
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
/// @ingroup dataaccess_tm
struct ISubtableInfoHolder : virtual public IHolder {

   /// @return a reference to the handler of the DATA_DESCRIPTION subtable
   virtual const ITableDataDescHolder& getDataDescription() const = 0;

   /// @return a reference to the handler of the SPECTRAL_WINDOW subtable
   virtual const ITableSpWindowHolder& getSpWindow() const = 0;

   /// @return a reference to the manager of buffers (BUFFERS subtable)
   virtual const IBufferManager& getBufferManager() const = 0;
   
   /// @return a reference to the handler of the FEED subtable
   virtual const IFeedSubtableHandler& getFeed() const = 0;
   
   /// @return a reference to the handler of the FIELD subtable
   virtual const IFieldSubtableHandler& getField() const = 0;

   /// @return a reference to the handler of the ANTENNA subtable
   virtual const IAntennaSubtableHandler& getAntenna() const = 0;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_SUBTABLE_INFO_HOLDER_H
