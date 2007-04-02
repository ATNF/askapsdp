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
//#include <casa/Quanta/MVDirection.h>

namespace conrad {

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
	virtual void chooseChannels(casa::uInt nChan,
	         casa::uInt start, casa::uInt nAvg) = 0;

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
       
};

} // end of namespace conrad

#endif /*I_DATA_SELECTOR_H*/
