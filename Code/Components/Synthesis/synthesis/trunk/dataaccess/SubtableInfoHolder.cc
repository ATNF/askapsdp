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

// casa includes
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/MemoryTable.h>
#include <tables/Tables/TableError.h>

// own includes
#include <dataaccess/SubtableInfoHolder.h>
#include <dataaccess/MemTableDataDescHolder.h>
#include <dataaccess/MemTableSpWindowHolder.h>
#include <dataaccess/TableBufferManager.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/FeedSubtableHandler.h>
#include <dataaccess/FieldSubtableHandler.h>

using namespace conrad;
using namespace conrad::synthesis;

/// @brief construct SubtableInfoHolder
/// @details The idea is that this constructor is the point where one can choose
/// how the lower level management is done (i.e. disk or memory based buffers). 
/// In the future, more arguments can be received by this constructor. It is probably
/// practical to provide reasonable defaults here
/// @param memBuffers true if the buffers should be held in memory, false if they should be
/// written back to the disk (table needs to be writable for this)
SubtableInfoHolder::SubtableInfoHolder(bool memBuffers) : itsUseMemBuffers(memBuffers) {}


/// @brief obtain data description holder
/// @details A MemTableDataDescHolder is constructed on the first call
/// to this method and a reference to it is always returned later
/// @return a reference to the handler of the DATA_DESCRIPTION subtable
const ITableDataDescHolder& SubtableInfoHolder::getDataDescription() const
{
  if (!itsDataDescHolder) {
      initDataDescHolder();
  }
  return *itsDataDescHolder;
}

/// @brief obtain spectral window holder
/// @details A MemTableSpWindowHolder is constructed on the first call
/// to this method and a reference to it is always returned later
/// @return a reference to the handler of the SPECTRAL_WINDOW subtable
const ITableSpWindowHolder& SubtableInfoHolder::getSpWindow() const
{
  if (!itsSpWindowHolder) {
      initSpWindowHolder();
  }
  return *itsSpWindowHolder;
}

/// initialize itsDataDescHolder with an instance of MemTableDataDescHolder.
void SubtableInfoHolder::initDataDescHolder() const
{
  itsDataDescHolder.reset(new MemTableDataDescHolder(table()));
}

/// initialize itsSpWindowHolder with an instance of MemTableSpWindowHolder.
void SubtableInfoHolder::initSpWindowHolder() const
{
  itsSpWindowHolder.reset(new MemTableSpWindowHolder(table()));
}

/// @brief obtain a manager of buffers
/// @details A TableBufferManager is constructed on the first call
/// to this method, which makes BUFFERS subtable if it is not yet
/// present.
/// @return a reference to the manager of buffers (BUFFERS subtable)
const IBufferManager& SubtableInfoHolder::getBufferManager() const
{
  if (!itsBufferManager) {
      initBufferManager();
  }
  return *itsBufferManager;
}

/// initialize itsBufferManager with an instance of TableBufferManager
void SubtableInfoHolder::initBufferManager() const
{  
  if (itsUseMemBuffers) {
      // After calling this method, the buffers will be held in
      // memory (via casa::MemoryTable), rather than be a subtable of
      // the measurement set.
      casa::SetupNewTable maker("BUFFERS",
								casa::TableDesc(),casa::Table::New);
      itsBufferManager.reset(new TableBufferManager(casa::Table(maker,
                                casa::Table::Memory)));      
  } else {
      // first, test that we have a compatible BUFFERS subtable if the
      // keyword exists
      if (table().keywordSet().isDefined("BUFFERS")) {
          try {
	     casa::Table testTab(table().keywordSet().asTable("BUFFERS"));
	     // just to avoid the compiler thinking that we don't use testTab
	     testTab.throwIfNull();
	  }
	  catch (...) {
	     // we have some problems with this subtable
	     try {
	        table().rwKeywordSet().removeField("BUFFERS");
	     }
	     catch (const casa::TableError &te) {
	        CONRADTHROW(DataAccessError,"Unable to remove corrupted BUFFERS keyword. AipsError: "<<
		            te.what());
	     }
	  }
      }
      if (!table().keywordSet().isDefined("BUFFERS")) {
          // we have to create a brand new subtable
          casa::SetupNewTable maker(table().tableName()+"/BUFFERS",
                                  casa::TableDesc(),casa::Table::New);
          table().rwKeywordSet().defineTable("BUFFERS",casa::Table(maker));
      }
      itsBufferManager.reset(new
               TableBufferManager(table().keywordSet().asTable("BUFFERS")));
  }
}

/// @brief obtain a feed subtable handler
/// @details A FeedSubtableHandler is constructred on the first call to
/// this method and a reference to it is always returne later
/// @return a reference to the handler of the FEED subtable
const IFeedSubtableHandler& SubtableInfoHolder::getFeed() const
{
  if (!itsFeedHolder) {
      initFeedHolder();
  }
  return *itsFeedHolder;
}

/// initialize itsFeedHolder with an instance of FeedSubtableHandler
void SubtableInfoHolder::initFeedHolder() const
{
  itsFeedHolder.reset(new FeedSubtableHandler(table()));  
}


/// @brief obtain a field subtable handler
/// @details A FieldSubtableHandler is consructed on the first call to this
/// method and a reference to it is returned thereafter.
/// @return a reference to the handler of the FIELD subtable
const IFieldSubtableHandler& SubtableInfoHolder::getField() const
{
  if (!itsFieldHolder) {
      initFieldHolder();
  }
  return *itsFieldHolder;
}

/// initialize itsFieldHolder with an instance of FieldSubtableHandler
void SubtableInfoHolder::initFieldHolder() const
{
  itsFieldHolder.reset(new FieldSubtableHandler(table()));
}
