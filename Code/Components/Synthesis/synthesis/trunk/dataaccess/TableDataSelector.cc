/// @file TableDataSelector.cc
///
/// TableBasedDataSelector: Class representing a selection of visibility
///                data according to some criterion. This is an
///                implementation of the part of the IDataSelector
///                interface, which can be done with the table selection
///                mechanism in the table based case 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// std include
#include <stdexcept>

/// own include
#include <dataaccess/TableDataSelector.h>

using namespace conrad;
using namespace synthesis;
using namespace casa;


/// construct a table selector with cycles defined by the time interval
/// @param tab MS table to work with
/// @param cycleInterval a duration of the cycle interval in seconds
///        Default value is 10 seconds.
///        (used to convert selection based on cycles to selection
///         using a normal time range)
TableDataSelector::TableDataSelector(const casa::Table &tab,
                                     casa::Double cycleInterval) :
                           itsMS(tab), itsUseRowsToGetCycles(false),
                           itsCycleInterval(cycleInterval) {}

/// construct a table selector with cycles defined by the given number of
/// rows. This is probably faster than the time based selection, but
/// requires all rows to be always present (i.e. all baselines should
/// always appear in the MS, even if some of them are fully flagged).
/// @param tab MS table to work with
/// @param cycleRows a number of rows per a cycle
///        (used to convert a selection based on cycles to the selection
///         using row numbers). This approach will only work if the number
///        of rows per cycle is fixed (i.e. always nBaselines x nFeeds)
TableDataSelector::TableDataSelector(const casa::Table &tab,
                                     casa::uInt cycleRows) :
         itsMS(tab), itsUseRowsToGetCycles(true), itsCycleRows(cycleRows) {}


/// Choose a single feed, the same for both antennae
/// @param feedID the sequence number of feed to choose
void TableDataSelector::chooseFeed(casa::uInt feedID)
{
   itsTableSelector=itsTableSelector && (itsMS.col("FEED") ==
                  static_cast<casa::Int>(feedID));
}

/// Choose a single baseline
/// @param ant1 the sequence number of the first antenna
/// @param ant2 the sequence number of the second antenna
/// Which one is the first and which is the second is not important
void TableDataSelector::chooseBaseline(casa::uInt ant1, casa::uInt ant2)
{
   itsTableSelector=itsTableSelector && (itsMS.col("ANTENNA1") ==
           static_cast<casa::Int>(ant1)) && (itsMS.col("ANTENNA2") ==
	   static_cast<casa::Int>(ant2));
}
  
/// Choose a time range. Both start and stop times are given via
/// casa::MVEpoch object. The reference frame is specified by
/// the DataSource object.
/// @param start the beginning of the chosen time interval
/// @param stop  the end of the chosen time interval
void TableDataSelector::chooseTimeRange(const casa::MVEpoch &start,
          const casa::MVEpoch &stop)
{
   throw std::runtime_error("not yet implemented");
}

/// Choose time range. This method accepts a time range with 
/// respect to the origin defined by the DataSource object.
/// Both start and stop times are given as Doubles.
/// The reference frame is the same as for the version accepting
/// MVEpoch and is specified via the DataSource object.
/// @param start the beginning of the chosen time interval
/// @param stop the end of the chosen time interval
void TableDataSelector::chooseTimeRange(casa::Double start,casa::Double stop)
{
   throw std::runtime_error("not yet implemented");
}
 
/// Choose cycles. This is an equivalent of choosing the time range,
/// but the selection is done in integer cycle numbers
/// @param start the number of the first cycle to choose
/// @param stop the number of the last cycle to choose
void TableDataSelector::chooseCycles(casa::uInt start, casa::uInt stop)
{
   throw std::runtime_error("not yet implemented");
}

/// Obtain a table expression node for selection. This method is
/// used in the implementation of the iterator to form a subtable
/// obeying the selection criteria specified by the user via
/// IDataSelector interface
///
/// @param conv  a reference to the converter, which is used to sort
///              out epochs used in the selection
const casa::TableExprNode& TableDataSelector::getTableSelector(
                                const IDataConverter &conv) const
{
   return itsTableSelector;
}
