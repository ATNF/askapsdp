/// @file IDataSelector.h
///
/// IDataSelector: Interface class prepresenting a selection of visibility
///                data according to some criterion. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_DATA_SELECTOR_H
#define I_DATA_SELECTOR_H

#include <casa/aips.h>
#include <casa/Quanta/MVFrequency.h>
#include <casa/Quanta/MVRadialVelocity.h>
#include <casa/Quanta/MVEpoch.h>
#include <casa/BasicSL/String.h>
//#include <casa/Quanta/MVDirection.h>

namespace conrad {

namespace synthesis {
	
// A derivative from this class is passed to a DataSource object in the
// request for an iterator. The iterator obtained that way runs through
// the selected part of the dataset (doesn't matter disk or stream based).
class IDataSelector
{
public:
	/// An empty virtual destructor to make the compiler happy
	virtual ~IDataSelector();
		
	/// Choose a single feed, the same for both antennae
	/// @param feedID the sequence number of feed to choose
	virtual void chooseFeed(casa::uInt feedID) = 0;

	/// Choose a single baseline
	/// @param ant1 the sequence number of the first antenna
	/// @param ant2 the sequence number of the second antenna
	/// Which one is the first and which is the second is not important
	virtual void chooseBaseline(casa::uInt ant1, casa::uInt ant2) = 0;
	
	/// Choose a subset of spectral channels
	/// @param nChan a number of spectral channels wanted in the output
	/// @param start the number of the first spectral channel to choose
	/// @param nAvg a number of adjacent spectral channels to average
	///             default is no averaging
	virtual void chooseChannels(casa::uInt nChan,
	         casa::uInt start, casa::uInt nAvg = 1) = 0;

	/// Choose a subset of frequencies. The reference frame is
	/// defined by the DataSource object
	/// @param nChan a number of spectral channels wanted in the output
	/// @param start the frequency of the first spectral channel to
	///        choose (given as casa::MVFrequency object)
	/// @param freqInc an increment in terms of the frequency in the
	///        same reference frame as start. This parameter plays
	///        the same role as nAvg for chooseChannels, i.e. twice
	///        the frequency resolution would average two adjacent channels
	virtual void chooseFrequencies(casa::uInt nChan,
	         const casa::MVFrequency &start, 
		 const casa::MVFrequency &freqInc) = 0;

	/// Choose a subset of radial velocities. The reference frame is
	/// defined by the DataSource object
	/// @param nChan a number of spectral channels wanted in the output
	/// @param start the velocity of the first spectral channel to
	///        choose (given as casa::MVRadialVelocity object)
	/// @param velInc an increment in terms of the radial velocity in the
	///        same reference frame as start. This parameter plays
	///        the same role as nAvg for chooseChannels, i.e. twice
	///        the velocity resolution would average two adjacent channels
	virtual void chooseVelocities(casa::uInt nChan,
	         const casa::MVRadialVelocity &start,
		 const casa::MVRadialVelocity &velInc) = 0;
	
	/// Choose a single spectral window (also known as IF).
	/// @param spWinID the ID of the spectral window to choose
	virtual void chooseSpectralWindow(casa::uInt spWinID) = 0;

	/// Choose a time range. The behavior for streams needs thinking.
	/// Probably the iterator should just ignore all data before the
	/// start time range and flags the end as soon as the time passed
	/// the stop time. Both start and stop times are given via
	/// casa::MVEpoch object. The reference frame is specified by
	/// the DataSource object.
	/// @param start the beginning of the chosen time interval
	/// @param stop  the end of the chosen time interval
	virtual void chooseTimeRange(const casa::MVEpoch &start,
	          const casa::MVEpoch &stop) = 0;

	/// Choose time range. This method accepts a time range with 
	/// respect to the origin defined by the DataSource object.
	/// Both start and stop times are given as Doubles.
	/// The reference frame is the same as for the version accepting
	/// MVEpoch and is specified via the DataSource object.
	/// @param start the beginning of the chosen time interval
	/// @param stop the end of the chosen time interval
	virtual void chooseTimeRange(casa::Double start,casa::Double stop) = 0;

	/// Choose polarization. 
	/// @param pols a string describing the wanted polarization 
	/// in the output. Allowed values are: I, "IQUV","XXYY","RRLL"
	virtual void choosePolarizations(const casa::String &pols) = 0;

	/// Choose cycles. This is an equivalent of choosing the time range,
	/// but the selection is done in integer cycle numbers
	/// @param start the number of the first cycle to choose
	/// @param stop the number of the last cycle to choose
	virtual void chooseCycles(casa::uInt start, casa::uInt stop) = 0;
};

} // end of namespace synthesis

} // end of namespace conrad

#endif /*I_DATA_SELECTOR_H*/
