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

/// own include
#include <dataaccess/TableDataSelector.h>
#include <dataaccess/DataAccessError.h>

using namespace conrad;
using namespace synthesis;
using namespace casa;


/// construct a table selector with cycles defined by the time interval
/// @param tab MS table to work with
TableDataSelector::TableDataSelector(const casa::Table &tab) :
                      TableScalarFieldSelector(tab) {}


/// Choose a time range. Both start and stop times are given via
/// casa::MVEpoch object. The reference frame is specified by
/// the DataSource object.
/// @param start the beginning of the chosen time interval
/// @param stop  the end of the chosen time interval
void TableDataSelector::chooseTimeRange(const casa::MVEpoch &start,
          const casa::MVEpoch &stop)
{
   throw DataAccessLogicError("not yet implemented");
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
   throw DataAccessLogicError("not yet implemented");
}
 
/// Choose cycles. This is an equivalent of choosing the time range,
/// but the selection is done in integer cycle numbers
/// @param start the number of the first cycle to choose
/// @param stop the number of the last cycle to choose
void TableDataSelector::chooseCycles(casa::uInt start, casa::uInt stop)
{
   throw DataAccessLogicError("not yet implemented");
}

/// Obtain a table expression node for selection. This method is
/// used in the implementation of the iterator to form a subtable
/// obeying the selection criteria specified by the user via
/// IDataSelector interface
///
/// @param conv  a reference to the converter, which is used to sort
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
/// @param nChan a number of spectral channels wanted in the output
/// @param start the number of the first spectral channel to choose
/// @param nAvg a number of adjacent spectral channels to average
///             default is no averaging
void TableDataSelector::chooseChannels(casa::uInt nChan, casa::uInt start,
                             casa::uInt nAvg)
{
   throw DataAccessLogicError("not yet implemented");
}

/// Choose a subset of frequencies. The reference frame is
/// defined by the DataSource object
/// @param nChan a number of spectral channels wanted in the output
/// @param start the frequency of the first spectral channel to
///        choose (given as casa::MVFrequency object)
/// @param freqInc an increment in terms of the frequency in the
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
/// @param nChan a number of spectral channels wanted in the output
/// @param start the velocity of the first spectral channel to
///        choose (given as casa::MVRadialVelocity object)
/// @param velInc an increment in terms of the radial velocity in the
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
/// @param pols a string describing the wanted polarization 
/// in the output. Allowed values are: I, "IQUV","XXYY","RRLL"
void TableDataSelector::choosePolarizations(const casa::String &pols)
{
   throw DataAccessLogicError("not yet implemented");
}

