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
	/// @param ref a reference frame to be used with all time epochs
	/// Default is UTC.
	virtual void setEpochFrame(const casa::MEpoch::Ref &ref) = 0;

	/// set the reference frame for any frequency
	/// (e.g. in the frequency-based selection or spectral labelling)
	/// @param ref a reference frame to be used with all frequencies
	/// Default is LSRK
	virtual void setFrequencyFrame(const casa::MFrequency::Ref &ref) = 0;

	/// set the reference frame for any velocity
	/// (e.g. in the velocity-based selection or spectral labelling)
	/// @param ref a reference frame to be used with all velocities
	/// Default is LSRK
	virtual void setVelocityFrame(const casa::MRadialVelocity::Ref &ref) = 0;
	
	/// get iterator over the whole dataset represented by this DataSource
	/// object
	virtual boost::shared_ptr<MEDataIterator> getIterator() const = 0;
	
	/// Initialize iteration with accessor
	/// @arg selection TaQL selection string
	void init(const string& selection);
	
	/// Initialize iteration with accessor
	void init();
	
	/// Return the data accessor
	MEDataAccessor& ida();
	
	/// Are there any more data?
	bool next() const;
	
protected:
private:
	METableDataAccessor itsIda;
};
}
#endif /*MEDATASOURCE_H_*/
