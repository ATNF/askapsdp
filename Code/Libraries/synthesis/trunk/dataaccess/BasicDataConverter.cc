/// @file
///
/// BasicDataConverter: An implementation of the data converter
/// (IDataConverter interface). The intention is to use it in conjunction
/// with the table based implementation of the data accessor. However,
/// it looks at this stage that this class is relatively general and can be
/// used with any implementation of the data accessor layer. One may want to
/// write a different implementation to achieve a better optimization, which
/// may be specific to a particular DataSource.
///
/// The main idea is to supply a DataConverter and DataSelector when
/// an iterator is requested from the DataSource object. The iterator will
/// return the data in the requested frame/units. The end user interacts
/// with the IDataConverter interface only.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// own includes
#include <dataaccess/BasicDataConverter.h>
#include <dataaccess/EpochConverter.h>
#include <dataaccess/DirectionConverter.h>

using namespace conrad;
using namespace synthesis;

BasicDataConverter::BasicDataConverter() :
     itsEpochConverter(new EpochConverter),
     itsDirectionConverter(new DirectionConverter)
{     
}

/// set the reference frame for any time epochs 
/// (e.g. time-based selection, visibility timestamp)
/// The value of the specified measure is the origin epoch. 
/// All visibility timestamps will be given as offsets from
/// it. The units of these offsets are given by the second
/// parameter
/// @param origin a zero-point for the visibility timestamps 
///        (they are given as time offsets with respect to 
///        this origin). A reference frame of this measure is
///        used in all time epochs (e.g. selection)
/// @param unit a required time unit for timestamps
///
/// Class defaults to MJD 0 UTC, timestamp in seconds
void BasicDataConverter::setEpochFrame(const casa::MEpoch &origin,
	   const casa::Unit &unit)
{
  itsEpochConverter.reset(new EpochConverter(origin,unit));
}

/// set the reference frame for directions. At this moment we
/// have only pointing direction accessible via DataAccessor.
/// In the future, selection based on the direction observed can
/// be added.
/// @param ref a reference frame to be used for all directions
///            (default is J2000).
/// @param unit units for all direction offsets. Unused at the
///             moment. Default units are radians.
void BasicDataConverter::setDirectionFrame(const casa::MDirection::Ref &ref,
               const casa::Unit &)
{
  itsDirectionConverter.reset(new DirectionConverter(ref));
}

/// set the reference frame for any frequency
/// (e.g. in the frequency-based selection or frequency to channel
///  mapping)
/// @param ref a reference frame to be used with all frequencies
/// @param unit frequency units to use (frequencies will be returned
///             as Doubles)
///
/// Class defaults to LSRK, GHz
void BasicDataConverter::setFrequencyFrame(const casa::MFrequency::Ref &ref,
       const casa::Unit &unit)
{
}

/// set the reference frame for any velocity
/// (e.g. in the velocity-based selection or spectral labelling)
/// @param ref a reference frame to be used with all velocities
/// @param unit velocity units to use (velocities will be returned
///             as Doubles)
///  
/// Class defaults to LSRK, km/s
void BasicDataConverter::setVelocityFrame(const casa::MRadialVelocity::Ref &ref,
		const casa::Unit &unit)
{
}

/// set the rest frequency required to do the frequency to velocity
/// conversion for most types of DataSources. Systems which
/// produce velocities directly (i.e. with a hardware Doppler
/// tracking) will require this if an operation with frequencies is
/// requested.
///
/// @param restFreq a rest frequency to be used for interconversions
///                 between frequencies and velocities
///
void BasicDataConverter::setRestFrequency(const casa::MVFrequency &restFreq)
{
}


/// set a frame (time, position), where the conversion is performed
void BasicDataConverter::setMeasFrame(const casa::MeasFrame &frame)
{
  itsEpochConverter->setMeasFrame(frame);
  itsDirectionConverter->setMeasFrame(frame);
}
