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
#include <dataaccess/ITableHolder.h>
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
struct SubtableInfoHolder : virtual public ISubtableInfoHolder,
                            virtual public ITableHolder
{

   /// @brief obtain data description holder
   /// @details A MemTableDataDescHolder is constructed on the first call
   /// to this method and a reference to it is always returned later
   /// @return a reference to the handler of the DATA_DESCRIPTION subtable
   virtual const ITableDataDescHolder& getDataDescription() const;

   /// @brief obtain spectral window holder
   /// @details A MemTableSpWindowHolder is constructed on the first call
   /// to this method and a reference to it is always returned later
   /// @return a reference to the handler of the SPECTRAL_WINDOW subtable
   virtual const ITableSpWindowHolder& getSpWindow() const;

   /// @brief obtain a manager of buffers
   /// @details A TableBufferManager is constructed on the first call
   /// to this method, which makes BUFFERS subtable if it is not yet
   /// present.
   /// @return a reference to the manager of buffers (BUFFERS subtable)
   virtual const IBufferManager& getBufferManager() const;

protected:   
   /// initialize itsDataDescHolder with an instance of MemTableDataDescHolder.
   void initDataDescHolder() const;

   /// initialize itsSpWindowHolder with an instance of MemTableSpWindowHolder.
   void initSpWindowHolder() const;

   /// initialize itsBufferManager with an instance of TableBufferManager
   void initBufferManager() const;
private:
   /// smart pointer to data description holder
   mutable boost::shared_ptr<ITableDataDescHolder const> itsDataDescHolder;

   /// smart pointer to spectral window holder
   mutable boost::shared_ptr<ITableSpWindowHolder const> itsSpWindowHolder;

   /// smart pointer to the buffer manager
   mutable boost::shared_ptr<IBufferManager const> itsBufferManager;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef SUBTABLE_INFO_HOLDER_H
