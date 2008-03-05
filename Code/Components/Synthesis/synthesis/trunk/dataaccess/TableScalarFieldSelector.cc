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
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// own include
#include <dataaccess/TableScalarFieldSelector.h>
#include <dataaccess/DataAccessError.h>
#include <tables/Tables/ExprNodeSet.h>


using namespace askap;
using namespace askap::synthesis;
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

/// @brief Choose samples corresponding to a uv-distance larger than threshold
/// @details This effectively rejects the baselines giving a smaller
/// uv-distance than the specified threshold
/// @param[in] uvDist threshold
void TableScalarFieldSelector::chooseMinUVDistance(casa::Double uvDist)
{
  TableExprNode uvwExprNode = table().col("UVW");
  const TableExprNode uExprNode = uvwExprNode(IPosition(1,0));
  const TableExprNode vExprNode = uvwExprNode(IPosition(1,1));
  
  if (itsTableSelector.isNull()) {
      itsTableSelector = (ndim(uvwExprNode) == 1) && (nelements(uvwExprNode) >= 2) 
                    && (sqrt(square(uExprNode)+square(vExprNode)) >= uvDist);
  } else {
      itsTableSelector = itsTableSelector && (ndim(uvwExprNode) == 1) && 
                 (nelements(uvwExprNode) >= 2) && 
                 (sqrt(square(uExprNode) + square(vExprNode)) >= uvDist);
  }
}

/// @brief Choose samples corresponding to a uv-distance smaller than threshold
/// @details This effectively rejects the baselines giving a larger
/// uv-distance than the specified threshold
/// @param[in] uvDist threshold
void TableScalarFieldSelector::chooseMaxUVDistance(casa::Double uvDist)
{
  TableExprNode uvwExprNode = table().col("UVW");
  const TableExprNode uExprNode = uvwExprNode(IPosition(1,0));
  const TableExprNode vExprNode = uvwExprNode(IPosition(1,1));
  
  if (itsTableSelector.isNull()) {
      itsTableSelector = (ndim(uvwExprNode) == 1) && (nelements(uvwExprNode) >= 2) 
                  && (sqrt(square(uExprNode) + square(vExprNode)) <= uvDist);
  } else {
      itsTableSelector = itsTableSelector && (ndim(uvwExprNode) == 1) &&
                 (nelements(uvwExprNode) > 2) && 
                 (sqrt(square(uExprNode) + square(vExprNode)) <= uvDist);
  }
}


/// @brief Choose autocorrelations only
void TableScalarFieldSelector::chooseAutoCorrelations()
{
   if (itsTableSelector.isNull()) {
       itsTableSelector = (table().col("ANTENNA1") ==
                           table().col("ANTENNA2")) &&
                           (table().col("FEED1") ==
                           table().col("FEED2"));
   } else {
       itsTableSelector = itsTableSelector && (table().col("ANTENNA1") ==
                                               table().col("ANTENNA2")) &&
                           (table().col("FEED1") == table().col("FEED2"));
   }
}
  
/// @brief Choose crosscorrelations only
void TableScalarFieldSelector::chooseCrossCorrelations()
{
   if (itsTableSelector.isNull()) {
       itsTableSelector = (table().col("ANTENNA1") !=
                           table().col("ANTENNA2")) || 
                           (table().col("FEED1") != table().col("FEED2"));
   } else {
       itsTableSelector = itsTableSelector && ((table().col("ANTENNA1") !=
                                               table().col("ANTENNA2")) ||
                           (table().col("FEED1") != table().col("FEED2"))) ;
   }
}

/// Choose a single spectral window (also known as IF).
/// @param spWinID the ID of the spectral window to choose
void TableScalarFieldSelector::chooseSpectralWindow(casa::uInt spWinID)
{
   // one spectral window can correspond to multiple data description IDs
   // We need to obtain this information from the DATA_DESCRIPTION table
   std::vector<size_t> dataDescIDs = subtableInfo().
        getDataDescription().getDescIDsForSpWinID(static_cast<int>(spWinID));
   if (dataDescIDs.size()) {
       std::vector<size_t>::const_iterator ci=dataDescIDs.begin();
       TableExprNode tempNode=(table().col("DATA_DESC_ID") ==
                  static_cast<casa::Int>(*ci));
       for(++ci;ci!=dataDescIDs.end();++ci) {           
	   tempNode = tempNode || (table().col("DATA_DESC_ID") ==
                  static_cast<casa::Int>(*ci));
       }
       if (itsTableSelector.isNull()) {
           itsTableSelector=tempNode;
       } else {
           itsTableSelector = itsTableSelector && tempNode;
       }       
   } else {
     // required spectral window is not present in the measurement set
     // we have to insert a dummy expression, otherwise an exception
     // is thrown within the table selection.
     itsTableSelector=(table().col("DATA_DESC_ID") == -1) && False;
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
