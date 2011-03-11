/// @file IDataConverter.h
/// @brief An interface to describe on-the-fly conversions
/// @details IDataConverter is an interface to describe on-the-fly
/// conversions requested
/// from the data source object. The polymorphism will allow a high performance
/// implementation in the future, i.e. bypassing conversionsi, if the data
/// appear in the requested frame/units up front. However, implementation of
/// this optimization will be deferred until the very latest stages. A
/// single converter class is expected to work for most of the cases.
///
/// The main idea is to supply a DataConverter and DataSelector when
/// an iterator is requested from the DataSource object. The iterator will
/// return the data in the requested frame/units.
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

#ifndef I_DATA_CONVERTER_H
#define I_DATA_CONVERTER_H

// CASA includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MRadialVelocity.h>
#include <casa/Quanta/MVFrequency.h>

// own includes
#include <dataaccess/IConverterBase.h>

namespace askap {

namespace accessors {

/// @brief An interface to describe on-the-fly conversions
/// @details IDataConverter is an interface to describe on-the-fly
/// conversions requested
/// from the data source object. The polymorphism will allow a high performance
/// implementation in the future, i.e. bypassing conversions, if the data
/// appear in the requested frame/units up front. However, implementation of
/// this optimization will be deferred until the very latest stages. A
/// single converter class is expected to work for most of the cases.
///
/// The main idea is to supply a DataConverter and DataSelector when
/// an iterator is requested from the DataSource object. The iterator will
/// return the data in the requested frame/units.	
/// @ingroup dataaccess_i
class IDataConverter : virtual public IConverterBase
{
public:
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
	virtual void setEpochFrame(const casa::MEpoch &origin = casa::MEpoch(),
		   const casa::Unit &unit = "s") = 0;

	/// set the reference frame for directions. At this moment we
	/// have only pointing direction accessible via DataAccessor.
	/// In the future, selection based on the direction observed can
	/// be added.
	/// @param ref a reference frame to be used for all directions
	///            (default is J2000).
	/// @param unit units for all direction offsets. Unused at the
	///             moment. Default units are radians.
	virtual void setDirectionFrame(const casa::MDirection::Ref &ref,
	               const casa::Unit &unit = "rad") = 0;

	/// set the reference frame for any frequency
	/// (e.g. in the frequency-based selection or frequency to channel
	///  mapping)
	/// @param ref a reference frame to be used with all frequencies
	/// @param unit frequency units to use (frequencies will be returned
	///             as Doubles)
	///
	/// Class defaults to LSRK, GHz
	virtual void setFrequencyFrame(const casa::MFrequency::Ref &ref,
		   const casa::Unit &unit = "GHz") = 0;

	/// set the reference frame for any velocity
	/// (e.g. in the velocity-based selection or spectral labelling)
	/// @param ref a reference frame to be used with all velocities
	/// @param unit velocity units to use (velocities will be returned
	///             as Doubles)
	///  
	/// Class defaults to LSRK, km/s
	virtual void setVelocityFrame(const casa::MRadialVelocity::Ref &ref,
			const casa::Unit &unit = "km/s") = 0;

        /// set the rest frequency required to do the frequency to velocity
	/// conversion for most types of DataSources. Systems which
	/// produce velocities directly (i.e. with a hardware Doppler
	/// tracking) will require this if an operation with frequencies is
	/// requested.
	///
	/// @param restFreq a rest frequency to be used for interconversions
	///                 between frequencies and velocities
	///
	virtual void setRestFrequency(const casa::MVFrequency &restFreq) = 0;
};

} // namespace accessors

} // namespace askap
#endif // I_DATA_CONVERTER_H
