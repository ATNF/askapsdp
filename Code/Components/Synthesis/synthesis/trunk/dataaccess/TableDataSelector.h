/// @file TableDataSelector.h
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
#ifndef TABLE_DATA_SELECTOR_H
#define TABLE_DATA_SELECTOR_H

/// casa includes
#include <tables/Tables/ExprNode.h>

/// own includes
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataConverter.h>

namespace conrad {

namespace synthesis {
	
// A derivative from this class is passed to a DataSource object in the
// request for an iterator. The iterator obtained that way runs through
// the selected part of the dataset.
class TableDataSelector : virtual public IDataSelector
{
public:
  /// Choose a single feed, the same for both antennae
  /// @param feedID the sequence number of feed to choose
  virtual void chooseFeed(casa::uInt feedID);

  /// Choose a single baseline
  /// @param ant1 the sequence number of the first antenna
  /// @param ant2 the sequence number of the second antenna
  /// Which one is the first and which is the second is not important
  virtual void chooseBaseline(casa::uInt ant1, casa::uInt ant2);
    
  /// Choose a time range. Both start and stop times are given via
  /// casa::MVEpoch object. The reference frame is specified by
  /// the DataSource object.
  /// @param start the beginning of the chosen time interval
  /// @param stop  the end of the chosen time interval
  virtual void chooseTimeRange(const casa::MVEpoch &start,
            const casa::MVEpoch &stop);
  
  /// Choose time range. This method accepts a time range with 
  /// respect to the origin defined by the DataSource object.
  /// Both start and stop times are given as Doubles.
  /// The reference frame is the same as for the version accepting
  /// MVEpoch and is specified via the DataSource object.
  /// @param start the beginning of the chosen time interval
  /// @param stop the end of the chosen time interval
  virtual void chooseTimeRange(casa::Double start,casa::Double stop);
   
  /// Choose cycles. This is an equivalent of choosing the time range,
  /// but the selection is done in integer cycle numbers
  /// @param start the number of the first cycle to choose
  /// @param stop the number of the last cycle to choose
  virtual void chooseCycles(casa::uInt start, casa::uInt stop);

  /// Obtain a table expression node for selection. This method is
  /// used in the implementation of the iterator to form a subtable
  /// obeying the selection criteria specified by the user via
  /// IDataSelector interface
  ///
  /// @param conv  a reference to the converter, which is used to sort
  ///              out epochs used in the selection
  const casa::TableExprNode& getTableSelector(const IDataConverter &conv) const;
public:
  /// a current table selection expression
  mutable casa::TableExprNode  itsTableSelector;
};
  
} // namespace synthesis
  
} // namespace conrad
  
#endif // #ifndef TABLE_DATA_SELECTOR_H
