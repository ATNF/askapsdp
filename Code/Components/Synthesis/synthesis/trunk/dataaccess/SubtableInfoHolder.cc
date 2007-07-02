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

// own includes
#include <dataaccess/SubtableInfoHolder.h>
#include <dataaccess/MemTableDataDescHolder.h>
#include <dataaccess/MemTableSpWindowHolder.h>
#include <dataaccess/TableBufferManager.h>
#include <dataaccess/DataAccessError.h>

using namespace conrad;
using namespace conrad::synthesis;

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
  if (!table().keywordSet().isDefined("BUFFERS")) {
      // we have to create a brand new subtable
      casa::SetupNewTable maker(table().tableName()+"/BUFFERS",
                                casa::TableDesc(),casa::Table::New);
      table().rwKeywordSet().defineTable("BUFFERS",casa::Table(maker));
  }
  itsBufferManager.reset(new
            TableBufferManager(table().keywordSet().asTable("BUFFERS")));
}

/// @brief set up BufferManager to be memory based.
/// @detail After calling this method, the buffers will be held in
/// memory (via casa::MemoryTable), rather than be a subtable of
/// the measurement set.
/// @note This method should be called before any operations with
/// the buffer
void SubtableInfoHolder::useMemoryBuffers() const
{
  CONRADASSERT(!itsBufferManager);
  casa::SetupNewTable maker("BUFFERS",
                            casa::TableDesc(),casa::Table::New);
  itsBufferManager.reset(new TableBufferManager(casa::Table(maker,
                         casa::Table::Memory)));
}
