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
/// @ingroup dataaccess_tm
struct SubtableInfoHolder : virtual public ISubtableInfoHolder,
                            virtual public ITableHolder
{
   /// @brief construct SubtableInfoHolder
   /// @details The idea is that this constructor is the point where one can choose
   /// how the lower level management is done (i.e. disk or memory based buffers). 
   /// In the future, more arguments can be received by this constructor. It is probably
   /// practical to provide reasonable defaults here
   /// @param memBuffers true if the buffers should be held in memory, false if they should be
   /// written back to the disk (table needs to be writable for this)
   explicit SubtableInfoHolder(bool memBuffers = false);

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
     
   /// @brief obtain a feed subtable handler
   /// @details A TableFeedHolder is constructred on the first call to
   /// this method and a reference to it is always returne later
   /// @return a reference to the handler of the FEED subtable
   virtual const ITableFeedHolder& getFeed() const;
   
   /// @brief obtain a field subtable handler
   /// @details A TableFieldHolder is consructed on the first call to this
   /// method and a reference to it is returned thereafter.
   /// @return a reference to the handler of the FIELD subtable
   virtual const ITableFieldHolder& getField() const;
   
protected:   
   /// initialize itsDataDescHolder with an instance of MemTableDataDescHolder.
   void initDataDescHolder() const;

   /// initialize itsSpWindowHolder with an instance of MemTableSpWindowHolder.
   void initSpWindowHolder() const;

   /// initialize itsBufferManager with an instance of TableBufferManager
   void initBufferManager() const;
   
   /// initialize itsFeedHolder with an instance of TableFeedHolder
   void initFeedHolder() const;
   
   /// initialize itsFieldHolder with an instance of TableFieldHolder
   void initFieldHolder() const;

private:
   /// smart pointer to data description holder
   mutable boost::shared_ptr<ITableDataDescHolder const> itsDataDescHolder;

   /// smart pointer to spectral window holder
   mutable boost::shared_ptr<ITableSpWindowHolder const> itsSpWindowHolder;

   /// smart pointer to the buffer manager
   mutable boost::shared_ptr<IBufferManager const> itsBufferManager;
   
   /// true if visibility buffers are kept in memory
   bool itsUseMemBuffers;

   /// smart pointer to the feed subtable handler
   mutable boost::shared_ptr<ITableFeedHolder const> itsFeedHolder;
   
   /// smart pointer to the field subtable handler
   mutable boost::shared_ptr<ITableFieldHolder const> itsFieldHolder;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef SUBTABLE_INFO_HOLDER_H
