/// @file
///
/// MEDataSource: Allow access to a source of visibility data, probably
/// either a MeasurementSet or a stream.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef MEDATASOURCE_H_
#define MEDATASOURCE_H_

// STL includes
#include <string>

// boost includes
#include <boost/shared_ptr.hpp>

// CASA includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MRadialVelocity.h>

// own includes
#include "MEDataIterator.h"
#include "IDataSelector.h"

using std::string;

#include "METableDataAccessor.h"

namespace conrad {
class MEDataSource
{
public:
	/// Construct a data source
	MEDataSource() {};
	
	/// empty virtual destructor to make the compiler happy
	virtual ~MEDataSource();

	/// set the reference frame for any time epochs 
	/// (e.g. time-based selection, visibility timestamp)
	/// The value of the specified measure is the origin epoch. 
	/// All visibility timestamps will be given as offsets from
	/// it. The units of these offsets are given by the second
	/// parameter
	/// @param origin a zero-point for the visibility timestamps 
	///        (they are given as time offsets with respect to 
	///        this origin). A reference frame of this measure is
	///        used all time epochs (e.g. selection)
	/// @param unit a required time unit for timestamps
	///
	/// Class defaults to MJD 0 UTC, timestamp in seconds
	virtual void setEpochFrame(const casa::MEpoch &ref,
		   const casa::Unit &unit) = 0;

	/// set the reference frame for any frequency
	/// (e.g. in the frequency-based selection or spectral labelling)
	/// @param ref a reference frame to be used with all frequencies
	/// @param unit frequency units to use (frequencies will be returned
	///             as Doubles)
	///
	/// Class defaults to LSRK, GHz
	virtual void setFrequencyFrame(const casa::MFrequency::Ref &ref,
		   const casa::Unit &unit) = 0;

	/// set the reference frame for any velocity
	/// (e.g. in the velocity-based selection or spectral labelling)
	/// @param ref a reference frame to be used with all velocities
	/// @param unit velocity units to use (velocities will be returned
	///             as Doubles)
	///  
	/// Class defaults to LSRK, km/s
	virtual void setVelocityFrame(const casa::MRadialVelocity::Ref &ref,
			const casa::Unit &unit) = 0;
	
	/// get iterator over the whole dataset represented by this DataSource
	/// object
	/// @return a shared pointer to DataIterator object
	virtual boost::shared_ptr<MEDataIterator> createIterator() const = 0;

	/// get iterator over a selected part of the dataset represented
	/// by this DataSource object
	/// @param[in] sel a shared pointer to the selector object defining 
	///            which subset of data is used
	/// @return a shared pointer to DataIterator object
	virtual boost::shared_ptr<MEDataIterator> createIterator(const
	           boost::shared_ptr<IDataSelector const> &sel) const = 0;

	/// create a selector object corresponding to this type of the
	/// DataSource
	/// @return a shared pointer to the DataSelector corresponding to
	/// this type of DataSource. DataSource acts as a factory and
	/// creates a selector object of the appropriate type
	virtual boost::shared_ptr<IDataSelector> createSelector() const = 0;
};
}
#endif /*MEDATASOURCE_H_*/
