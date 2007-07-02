/// @file TableDataSelector.h
/// @brief Implementation of IDataSelector is the table-based case
/// @details
/// TableDataSelector: Class representing a selection of visibility
///                data according to some criterion. This is an
///                implementation of the IDataSelector interface 
///                in the table-based case. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_DATA_SELECTOR_H
#define TABLE_DATA_SELECTOR_H

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes

// own includes
#include <dataaccess/TableScalarFieldSelector.h>
#include <dataaccess/IDataConverterImpl.h>
#include <dataaccess/ITableMeasureFieldSelector.h>
#include <dataaccess/TableInfoAccessor.h>
#include <dataaccess/ITableManager.h>

namespace conrad {

namespace synthesis {
	
/// @brief Implementation of IDataSelector is the table-based case
/// @details
/// TableDataSelector: Class representing a selection of visibility
///                data according to some criterion. This is an
///                implementation of the IDataSelector interface 
///                in the table-based case. 
class TableDataSelector : public TableScalarFieldSelector,
                          virtual protected TableInfoAccessor
{
public:
  /// construct a table selector passing a table/derived info manager
  /// via a smart pointer 
  /// @param[in] msManager a shared pointer to the manager of the measurement set
  /// (a derivative of ISubtableInfoHolder)
  ///
  explicit TableDataSelector(const boost::shared_ptr<ITableManager const> &msManager);
   
  /// Choose a time range. Both start and stop times are given via
  /// casa::MVEpoch object. The reference frame is specified by
  /// the DataSource object.
  /// @param[in] start the beginning of the chosen time interval
  /// @param[in] stop  the end of the chosen time interval
  virtual void chooseTimeRange(const casa::MVEpoch &start,
            const casa::MVEpoch &stop);
  
  /// Choose time range. This method accepts a time range with 
  /// respect to the origin defined by the DataSource object.
  /// Both start and stop times are given as Doubles.
  /// The reference frame is the same as for the version accepting
  /// MVEpoch and is specified via the DataSource object.
  /// @param[in] start the beginning of the chosen time interval
  /// @param[in] stop the end of the chosen time interval
  virtual void chooseTimeRange(casa::Double start,casa::Double stop);
   
  /// Choose cycles. This is an equivalent of choosing the time range,
  /// but the selection is done in integer cycle numbers
  /// @param[in] start the number of the first cycle to choose
  /// @param[in] stop the number of the last cycle to choose
  virtual void chooseCycles(casa::uInt start, casa::uInt stop);

  /// Choose a subset of spectral channels
  /// @param[in] nChan a number of spectral channels wanted in the output
  /// @param[in] start the number of the first spectral channel to choose
  /// @param[in] nAvg a number of adjacent spectral channels to average
  ///             default is no averaging
  virtual void chooseChannels(casa::uInt nChan, casa::uInt start,
                               casa::uInt nAvg = 1);

  /// Choose a subset of frequencies. The reference frame is
  /// defined by the DataSource object
  /// @param[in] nChan a number of spectral channels wanted in the output
  /// @param[in] start the frequency of the first spectral channel to
  ///        choose (given as casa::MVFrequency object)
  /// @param[in] freqInc an increment in terms of the frequency in the
  ///        same reference frame as start. This parameter plays
  ///        the same role as nAvg for chooseChannels, i.e. twice
  ///        the frequency resolution would average two adjacent channels
  virtual void chooseFrequencies(casa::uInt nChan,
           const casa::MVFrequency &start, const casa::MVFrequency &freqInc);

  /// Choose a subset of radial velocities. The reference frame is
  /// defined by the DataSource object
  /// @param[in] nChan a number of spectral channels wanted in the output
  /// @param[in] start the velocity of the first spectral channel to
  ///        choose (given as casa::MVRadialVelocity object)
  /// @param[in] velInc an increment in terms of the radial velocity in the
  ///        same reference frame as start. This parameter plays
  ///        the same role as nAvg for chooseChannels, i.e. twice
  ///        the velocity resolution would average two adjacent channels
  virtual void chooseVelocities(casa::uInt nChan,
           const casa::MVRadialVelocity &start,
  	 const casa::MVRadialVelocity &velInc);

  /// Choose polarization. 
  /// @param[in] pols a string describing the wanted polarization 
  /// in the output. Allowed values are: I, "IQUV","XXYY","RRLL"
  virtual void choosePolarizations(const casa::String &pols);

  /// Obtain a table expression node for selection. This method is
  /// used in the implementation of the iterator to form a subtable
  /// obeying the selection criteria specified by the user via
  /// IDataSelector interface
  ///
  /// @param[in] conv  a shared pointer to the converter, which is used to sort
  ///              out epochs and other measures used in the selection
  virtual const casa::TableExprNode& getTableSelector(const
                  boost::shared_ptr<IDataConverterImpl const> &conv) const;
private:
  /// a measurement set to work with. Reference semantics
  casa::Table itsMS;
  boost::shared_ptr<ITableMeasureFieldSelector> itsEpochSelector;
};
  
} // namespace synthesis
  
} // namespace conrad
  
#endif // #ifndef TABLE_DATA_SELECTOR_H
