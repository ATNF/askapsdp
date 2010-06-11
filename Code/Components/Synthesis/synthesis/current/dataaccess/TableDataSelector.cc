/// @file TableDataSelector.cc
/// @brief Implementation of IDataSelector is the table-based case
/// @details
/// TableBasedDataSelector: Class representing a selection of visibility
///                data according to some criterion. This is an
///                implementation of the part of the IDataSelector
///                interface, which can be done with the table selection
///                mechanism in the table based case 
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

/// own include
#include <dataaccess/TableDataSelector.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableTimeStampSelectorImpl.h>

using namespace askap;
using namespace askap::synthesis;
using namespace casa;


/// construct a table selector passing a table/derived info manager
/// via a smart pointer 
/// @param[in] msManager a shared pointer to the manager of the measurement set
/// (a derivative of ISubtableInfoHolder)
TableDataSelector::TableDataSelector(const
       boost::shared_ptr<ITableManager const> &msManager) :
       TableInfoAccessor(msManager), 
#ifndef ASKAP_DEBUG
       itsDataColumnName(msManager->defaultDataColumnName()),
#endif       
       itsChannelSelection(-1,0)
{
  ASKAPDEBUGASSERT(msManager);
#ifdef ASKAP_DEBUG
  itsDataColumnName = msManager->defaultDataColumnName();
#endif 
}

/// Choose a time range. Both start and stop times are given via
/// casa::MVEpoch object. The reference frame is specified by
/// the DataSource object.
/// @param[in] start the beginning of the chosen time interval
/// @param[in] stop  the end of the chosen time interval
void TableDataSelector::chooseTimeRange(const casa::MVEpoch &start,
          const casa::MVEpoch &stop)
{
   itsEpochSelector.reset(new TableTimeStampSelectorImpl<casa::MVEpoch>(table(),
                          start, stop));
}

/// Choose time range. This method accepts a time range with 
/// respect to the origin defined by the DataSource object.
/// Both start and stop times are given as Doubles.
/// The reference frame is the same as for the version accepting
/// MVEpoch and is specified via the DataSource object.
/// @param[in] start the beginning of the chosen time interval
/// @param[in] stop the end of the chosen time interval
void TableDataSelector::chooseTimeRange(casa::Double start,casa::Double stop)
{
   itsEpochSelector.reset(new TableTimeStampSelectorImpl<casa::Double>(table(),
                          start, stop));
}
 
/// Choose cycles. This is an equivalent of choosing the time range,
/// but the selection is done in integer cycle numbers
/// @param[in] start the number of the first cycle to choose
/// @param[in] stop the number of the last cycle to choose
void TableDataSelector::chooseCycles(casa::uInt start, casa::uInt stop)
{
   throw DataAccessLogicError("not yet implemented");
}

/// Obtain a table expression node for selection. This method is
/// used in the implementation of the iterator to form a subtable
/// obeying the selection criteria specified by the user via
/// IDataSelector interface
///
/// @param[in] conv  a reference to the converter, which is used to sort
///              out epochs used in the selection
const casa::TableExprNode& TableDataSelector::getTableSelector(const
                    boost::shared_ptr<IDataConverterImpl const> &conv) const
{  
   if (itsEpochSelector) { 
       /// epoch selection has been done, we need to narrow down the
       /// selection by updating the table expression
       itsEpochSelector->setConverter(conv);
       itsEpochSelector->updateTableExpression(
                    TableScalarFieldSelector::getTableSelector());
   }
   return TableScalarFieldSelector::getTableSelector();
}

/// Choose a subset of spectral channels
/// @param[in] nChan a number of spectral channels wanted in the output
/// @param[in] start the number of the first spectral channel to choose
/// @param[in] nAvg a number of adjacent spectral channels to average
///             default is no averaging
void TableDataSelector::chooseChannels(casa::uInt nChan, casa::uInt start,
                             casa::uInt nAvg)
{
   if (nAvg != 1) {
       throw DataAccessLogicError("not yet implemented");
   }
   ASKAPDEBUGASSERT((nChan>0) && (start>=0)); 
   itsChannelSelection.first = int(nChan);
   itsChannelSelection.second = int(start);
}

/// Choose a subset of frequencies. The reference frame is
/// defined by the DataSource object
/// @param[in] nChan a number of spectral channels wanted in the output
/// @param[in] start the frequency of the first spectral channel to
///        choose (given as casa::MVFrequency object)
/// @param[in] freqInc an increment in terms of the frequency in the
///        same reference frame as start. This parameter plays
///        the same role as nAvg for chooseChannels, i.e. twice
///        the frequency resolution would average two adjacent channels
void TableDataSelector::chooseFrequencies(casa::uInt nChan,
         const casa::MVFrequency &start, const casa::MVFrequency &freqInc)
{
   throw DataAccessLogicError("not yet implemented");
}

/// Choose a subset of radial velocities. The reference frame is
/// defined by the DataSource object
/// @param[in] nChan a number of spectral channels wanted in the output
/// @param[in] start the velocity of the first spectral channel to
///        choose (given as casa::MVRadialVelocity object)
/// @param[in] velInc an increment in terms of the radial velocity in the
///        same reference frame as start. This parameter plays
///        the same role as nAvg for chooseChannels, i.e. twice
///        the velocity resolution would average two adjacent channels
void TableDataSelector::chooseVelocities(casa::uInt nChan,
         const casa::MVRadialVelocity &start,
	 const casa::MVRadialVelocity &velInc)
{
   throw DataAccessLogicError("not yet implemented");
}

/// Choose polarization. 
/// @param[in] pols a string describing the wanted polarization 
/// in the output. Allowed values are: I, "IQUV","XXYY","RRLL"
void TableDataSelector::choosePolarizations(const casa::String &pols)
{
   throw DataAccessLogicError("not yet implemented");
}

/// @brief choose data column
/// @details This method allows to choose any table column as the visibility
/// data column (e.g. DATA, CORRECTED_DATA, etc). Because this is a
/// table-specific operation, this method is defined in a table-specific
/// selector interface and is not present in IDataSelector (therefore,
/// a dynamic_pointer_cast is likely required).
/// @param[in] dataColumn column name, which contains visibility data 
void TableDataSelector::chooseDataColumn(const std::string &dataColumn)
{
   itsDataColumnName = dataColumn;
}  

/// @brief clone a selector
/// @details The same selector can be used to create a number of iterators.
/// Selector stores a name of the data column to use and, therefore, it can
/// be changed after some iterators are created. To avoid bugs due to this
/// reference semantics, the iterator will clone selector in its constructor.
/// @note This functionality is not exposed to the end user, which
/// normally interacts with the IDataSelector class only. This is because
/// cloning is done at the low level (e.g. inside the iterator)
boost::shared_ptr<ITableDataSelectorImpl const> TableDataSelector::clone() const
{
  return boost::shared_ptr<ITableDataSelectorImpl const>(new TableDataSelector(*this));
}

/// @brief obtain the name of data column
/// @details This method returns the current name of the data column, set 
/// either in the constructor or by the chooseDataColumn method
/// @return the name of the data column
const std::string& TableDataSelector::getDataColumnName() const throw() 
{
  return itsDataColumnName;
}

/// @brief check whether channel selection has been done
/// @details By default all channels are selected. However, if chooseChannels 
/// has been called, less channels are returned. This method returns true if
/// this is the case and false otherwise.
/// @return true, if a subset of channels has been selected
bool TableDataSelector::channelsSelected() const throw()
{
  return itsChannelSelection.first >= 0;
}
  
/// @brief obtain channel selection
/// @details By default all channels are selected. However, if chooseChannels 
/// has been called, less channels are returned by the accessor. This method
/// returns the number of channels and the first channel (in the full sample) 
/// selected. If the first element of the pair is negative, no channel-based
/// selection has been done. This is also checked by channelsSelected method, 
/// which is probably a prefered way to do this check to retain the code clarity.
/// @return a pair, the first element gives the number of channels selected and
/// the second element gives the start channel (0-based)
std::pair<int,int> TableDataSelector::getChannelSelection() const throw()
{
  return itsChannelSelection;
}

