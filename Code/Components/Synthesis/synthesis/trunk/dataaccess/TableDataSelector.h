/// @file TableDataSelector.h
/// @brief Implementation of IDataSelector is the table-based case
/// @details
/// TableDataSelector: Class representing a selection of visibility
///                data according to some criterion. This is an
///                implementation of the IDataSelector interface 
///                in the table-based case. 
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_DATA_SELECTOR_H
#define TABLE_DATA_SELECTOR_H

// std includes
#include <string>

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes

// own includes
#include <dataaccess/TableScalarFieldSelector.h>
#include <dataaccess/IDataConverterImpl.h>
#include <dataaccess/ITableMeasureFieldSelector.h>
#include <dataaccess/TableInfoAccessor.h>
#include <dataaccess/ITableManager.h>

namespace askap {

namespace synthesis {
	
/// @brief Implementation of IDataSelector is the table-based case
/// @details
/// TableDataSelector: Class representing a selection of visibility
///                data according to some criterion. This is an
///                implementation of the IDataSelector interface 
///                in the table-based case. 
/// @ingroup dataaccess_tab
class TableDataSelector : public TableScalarFieldSelector,
                          virtual protected TableInfoAccessor
{
public:
  /// construct a table selector passing a table/derived info manager
  /// via a smart pointer 
  /// @param[in] msManager a shared pointer to the manager of the measurement set
  /// (a derivative of ISubtableInfoHolder)
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

  /// @brief choose data column
  /// @details This method allows to choose any table column as the visibility
  /// data column (e.g. DATA, CORRECTED_DATA, etc). Because this is a
  /// table-specific operation, this method is defined in a table-specific
  /// selector interface and is not present in IDataSelector (therefore,
  /// a dynamic_pointer_cast is likely required).
  /// @param[in] dataColumn column name, which contains visibility data 
  virtual void chooseDataColumn(const std::string &dataColumn);  

  /// @brief obtain the name of data column
  /// @details This method returns the current name of the data column, 
  /// set either in the constructor or by the chooseDataColumn method
  /// @return the name of the data column
  virtual const std::string& getDataColumnName() const throw();

  /// @brief clone a selector
  /// @details The same selector can be used to create a number of iterators.
  /// Selector stores a name of the data column to use and, therefore, it can
  /// be changed after some iterators are created. To avoid bugs due to this
  /// reference semantics, the iterator will clone selector in its constructor.
  /// @note This functionality is not exposed to the end user, which
  /// normally interacts with the IDataSelector class only. This is because
  /// cloning is done at the low level (e.g. inside the iterator)
  virtual boost::shared_ptr<ITableDataSelectorImpl const> clone() const;

private:
  /// a measurement set to work with. Reference semantics
  casa::Table itsMS;
  /// selector for epoch
  boost::shared_ptr<ITableMeasureFieldSelector> itsEpochSelector;
    /// a name of the column containing visibility data
  std::string itsDataColumnName; 
};
  
} // namespace synthesis
  
} // namespace askap
  
#endif // #ifndef TABLE_DATA_SELECTOR_H
