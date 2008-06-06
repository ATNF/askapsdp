/// @file BasicDataConverter.cc
/// @brief An implementation of the data converter
/// @details
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

// own includes
#include <dataaccess/BasicDataConverter.h>
#include <dataaccess/EpochConverter.h>
#include <dataaccess/DirectionConverter.h>
#include <dataaccess/DopplerConverter.h>
#include <dataaccess/DataAccessError.h>

using namespace askap;
using namespace askap::synthesis;

BasicDataConverter::BasicDataConverter() :
     itsEpochConverter(new EpochConverter),
     itsDirectionConverter(new DirectionConverter),
     itsFrequencyConverter(new GenericConverter<casa::MFrequency>(
                           casa::MFrequency::Ref(casa::MFrequency::LSRK),
			   "GHz")),
     itsVelocityConverter(new GenericConverter<casa::MRadialVelocity>(
                     casa::MRadialVelocity::Ref(casa::MRadialVelocity::LSRK),
		     "km/s"))
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
  itsFrequencyConverter.reset(new GenericConverter<casa::MFrequency>(ref,unit));
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
  itsVelocityConverter.reset(new GenericConverter<casa::MRadialVelocity>(ref,unit));
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
  itsDopplerConverter.reset(new DopplerConverter(restFreq,
                            casa::MDoppler::RADIO));
}


/// set a frame (time, position), where the conversion is performed
void BasicDataConverter::setMeasFrame(const casa::MeasFrame &frame)
{
  itsEpochConverter->setMeasFrame(frame);
  itsDirectionConverter->setMeasFrame(frame);
  itsFrequencyConverter->setMeasFrame(frame);
  itsVelocityConverter->setMeasFrame(frame);
}

/// test whether the frequency conversion is void
/// @param[in] testRef reference frame to test
/// @param[in] testUnit units to test
bool BasicDataConverter::isVoid(const casa::MFrequency::Ref &testRef,
                    const casa::Unit &testUnit) const
{
  return itsFrequencyConverter->isVoid(testRef,testUnit);
}

/// convert epochs
/// @param in input epoch given as an MEpoch object
/// @return epoch converted to Double 
casa::Double BasicDataConverter::epoch(const casa::MEpoch &in) const
{
  return (*itsEpochConverter)(in);
}

/// reverse conversion: form a measure from 'double' epoch
/// @param[in] in epoch given as Double in the target units/frame
/// @return epoch converted to Measure
casa::MEpoch BasicDataConverter::epochMeasure(casa::Double in) const
{
  return itsEpochConverter->toMeasure(in);
}

/// reverse conversion: form a measure from MVEpoch
/// @param[in] in epoch given as MVEpoch in the target frame
/// @return epoch converted to Measure
casa::MEpoch BasicDataConverter::epochMeasure(const casa::MVEpoch &in) const
{
  return itsEpochConverter->toMeasure(in);
}

/// convert directions
/// @param in input direction given as an MDirection object
/// @param out output direction as an MVDirection object
void BasicDataConverter::direction(const casa::MDirection &in,
                      casa::MVDirection &out) const
{
  out=(*itsDirectionConverter)(in);
}

/// convert frequencies
/// @param in input frequency given as an MFrequency object
/// @param out output frequency as a Double
casa::Double BasicDataConverter::frequency(const casa::MFrequency &in) const
{
  return (*itsFrequencyConverter)(in);
}

/// convert velocities
/// @param in input velocities given as an MRadialVelocity object
/// @param out output velocity as a Double
casa::Double BasicDataConverter::velocity(const casa::MRadialVelocity &in)
                                          const
{
  return (*itsVelocityConverter)(in);
}

/// convert frequencies from velocities
/// @param in input velocity given as an MRadialVelocity object
/// @param out output frequency as a Double
///
/// Note, an exception will be thrown if the rest frequency is not
/// defined.
///
casa::Double BasicDataConverter::frequency(const casa::MRadialVelocity &in)
                                           const
{      
  if (!itsDopplerConverter) {
      throw DataAccessLogicError("A rest frequency is needed to be able to "
      "use BasicDataConverter::frequency(MRadialVelocity)");
  }
  return (*itsFrequencyConverter)((*itsDopplerConverter)(in));
}

/// convert velocities from frequencies
/// @param in input frequency  given as an MFrequency object
/// @param out output velocity as a Double
///
/// Note, an exception will be thrown if the rest frequency is not
/// defined.
///
casa::Double BasicDataConverter::velocity(const casa::MFrequency &in) const
{      
  if (!itsDopplerConverter) {
      throw DataAccessLogicError("A rest frequency is needed to be able to "
      "use BasicDataConverter::frequency(MRadialVelocity)");
  }      
  return (*itsVelocityConverter)((*itsDopplerConverter)(in));
}

/// @brief Clone the converter (sort of a virtual constructor
/// @details The same converter can be used to create a number of iterators.
/// However, we need to set the reference frame to perform some 
/// conversions on the per-iterator basis. To avoid very nasty bugs when 
/// two independent iterators indirectly affect each other by using different
/// reference frames for conversion, it is practical to isolate all changes
/// to a private copy of the converter. Each iterator will clone a 
/// converter in the constructor instead of using the same instance via
/// a smart pointer. 
/// @note An alternative approach is to ammend the interface
/// to pass the reference frame as an argument for methods, which perform
/// the conversions. Time will show which one is better. A change from one
/// way of solving the problem to another doesn't affect the high level
/// user interface and can be done relatively easy.
/// @return a smart pointer to a clone of this instance of the converter
boost::shared_ptr<IDataConverterImpl> BasicDataConverter::clone() const
{
  return boost::shared_ptr<IDataConverterImpl>(new BasicDataConverter(*this));
}  
