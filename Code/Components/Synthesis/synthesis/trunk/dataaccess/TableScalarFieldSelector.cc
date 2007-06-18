/// @file TableScalarFieldSelector.cc
/// @brief An implementation of ITableDataSelectorImpl for simple (scalar) fields, like feed ID.
/// @details This class represents a selection of visibility
///         data according to some criterion. This is an
///         implementation of the part of the IDataSelector
///         interface, which can be done with the table selection
///         mechanism in the table based case. Only simple
///         (scalar) fields are included in this selection.
///         Epoch-based selection is done via a separate class
///         because a fully defined converter is required to
///         perform such selection.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own include
#include <dataaccess/TableScalarFieldSelector.h>
#include <dataaccess/DataAccessError.h>

using namespace conrad;
using namespace synthesis;
using namespace casa;


/// Choose a single feed, the same for both antennae
/// @param feedID the sequence number of feed to choose
void TableScalarFieldSelector::chooseFeed(casa::uInt feedID)
{
   if (itsTableSelector.isNull()) {
       itsTableSelector= (table().col("FEED1") ==
                  static_cast<casa::Int>(feedID)) && (table().col("FEED2") ==
                  static_cast<casa::Int>(feedID));
   } else {
       itsTableSelector=itsTableSelector && (table().col("FEED1") ==
                  static_cast<casa::Int>(feedID)) && (table().col("FEED2") ==
                  static_cast<casa::Int>(feedID));
   }
}

/// Choose a single baseline
/// @param ant1 the sequence number of the first antenna
/// @param ant2 the sequence number of the second antenna
/// Which one is the first and which is the second is not important
void TableScalarFieldSelector::chooseBaseline(casa::uInt ant1,
                                              casa::uInt ant2)
{
   if (itsTableSelector.isNull()) {
       itsTableSelector= (table().col("ANTENNA1") ==
           static_cast<casa::Int>(ant1)) && (table().col("ANTENNA2") ==
	   static_cast<casa::Int>(ant2));
   } else {
       itsTableSelector=itsTableSelector && (table().col("ANTENNA1") ==
           static_cast<casa::Int>(ant1)) && (table().col("ANTENNA2") ==
	   static_cast<casa::Int>(ant2));
   }
}

/// Choose a single spectral window (also known as IF).
/// @param spWinID the ID of the spectral window to choose
void TableScalarFieldSelector::chooseSpectralWindow(casa::uInt spWinID)
{
   /// @todo we need a proper spectral window selection, which works with
   /// the structure of the measurement set to extract data description ids
   /// required for this selection
   
   if (itsTableSelector.isNull()) {
       itsTableSelector=(table().col("DATA_DESC_ID") ==
                  static_cast<casa::Int>(spWinID));
   } else {
       itsTableSelector=itsTableSelector && (table().col("DATA_DESC_ID") ==
                  static_cast<casa::Int>(spWinID));
   }
}
 
/// Obtain a table expression node for selection. This method is
/// used in the implementation of the iterator to form a subtable
/// obeying the selection criteria specified by the user via
/// IDataSelector interface
///
/// @return a reference to the cached table expression node
///
casa::TableExprNode& TableScalarFieldSelector::getTableSelector() const
{ 
  return itsTableSelector;
}
