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

// own includes
#include <dataaccess/SubtableInfoHolder.h>
#include <dataaccess/MemTableDataDescHolder.h>
#include <dataaccess/MemTableSpWindowHolder.h>

using namespace conrad;
using namespace synthesis;

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

