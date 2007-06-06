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

#ifndef BASIC_DATA_CONVERTER_H
#define BASIC_DATA_CONVERTER_H

// std includes
#include <stdexcept>

// boost includes
#include <boost/shared_ptr.hpp>

// CASA includes
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MRadialVelocity.h>
// following includes are required to instantiate a generic converter
// for MRadialVelocity and MFrequency
#include <measures/Measures/MCRadialVelocity.h>
#include <measures/Measures/MCFrequency.h>

// own includes
#include <dataaccess/IDataConverter.h>
#include <dataaccess/IEpochConverter.h>
#include <dataaccess/IDirectionConverter.h>
#include <dataaccess/GenericConverter.h>
#include <dataaccess/IDopplerConverter.h>

namespace conrad {

namespace synthesis {
	
class BasicDataConverter : public IDataConverter
{
public:
    /// default constructor sets up the default conversion options,
    /// which are:
    ///    for Epoch, origin/frame are MJD 0 UTC, units are seconds
    ///               (defined in the default arguments for the
    ///                constructor of EpochConverter)
    ///    for Directions, frame is J2000, units are not used
    BasicDataConverter();

    /// implementation of the interface methods

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
    	   const casa::Unit &unit = "s");
    
    /// set the reference frame for directions. At this moment we
    /// have only pointing direction accessible via DataAccessor.
    /// In the future, selection based on the direction observed can
    /// be added.
    /// @param ref a reference frame to be used for all directions
    ///            (default is J2000).
    /// @param unit units for all direction offsets. Unused at the
    ///             moment. Default units are radians.
    virtual void setDirectionFrame(const casa::MDirection::Ref &ref,
                   const casa::Unit &unit = "rad");
    
    /// set the reference frame for any frequency
    /// (e.g. in the frequency-based selection or frequency to channel
    ///  mapping)
    /// @param ref a reference frame to be used with all frequencies
    /// @param unit frequency units to use (frequencies will be returned
    ///             as Doubles)
    ///
    /// Class defaults to LSRK, GHz
    virtual void setFrequencyFrame(const casa::MFrequency::Ref &ref,
	   const casa::Unit &unit = "GHz");

    /// set the reference frame for any velocity
    /// (e.g. in the velocity-based selection or spectral labelling)
    /// @param ref a reference frame to be used with all velocities
    /// @param unit velocity units to use (velocities will be returned
    ///             as Doubles)
    ///  
    /// Class defaults to LSRK, km/s
    virtual void setVelocityFrame(const casa::MRadialVelocity::Ref &ref,
    		const casa::Unit &unit = "km/s");
    
    /// set the rest frequency required to do the frequency to velocity
    /// conversion for most types of DataSources. Systems which
    /// produce velocities directly (i.e. with a hardware Doppler
    /// tracking) will require this if an operation with frequencies is
    /// requested.
    ///
    /// @param restFreq a rest frequency to be used for interconversions
    ///                 between frequencies and velocities
    ///
    virtual void setRestFrequency(const casa::MVFrequency &restFreq);

    /// set a frame (for epochs it is just a position), where the
    /// conversion is performed
    virtual void setMeasFrame(const casa::MeasFrame &frame);


    /// following methods are used within the DataSource/DataIterator
    
    /// convert epochs
    /// @param in input epoch given as an MEpoch object
    /// @return epoch converted to Double 
    casa::Double inline epoch(const casa::MEpoch &in) const
    {
      return (*itsEpochConverter)(in);
    }

    /// convert directions
    /// @param in input direction given as an MDirection object
    /// @param out output direction as an MVDirection object
    void inline direction(const casa::MDirection &in,
                          casa::MVDirection &out) const
    {
      out=(*itsDirectionConverter)(in);
    }

    /// convert frequencies
    /// @param in input frequency given as an MFrequency object
    /// @param out output frequency as a Double
    casa::Double inline frequency(const casa::MFrequency &in) const
    {
      return (*itsFrequencyConverter)(in);
    }

    /// convert velocities
    /// @param in input velocities given as an MRadialVelocity object
    /// @param out output velocity as a Double
    casa::Double inline velocity(const casa::MRadialVelocity &in) const
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
    casa::Double inline frequency(const casa::MRadialVelocity &in) const
    {      
      if (!itsDopplerConverter) {
          throw std::logic_error("A rest frequency is needed to be able to "
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
    casa::Double inline velocity(const casa::MFrequency &in) const
    {      
      if (!itsDopplerConverter) {
          throw std::logic_error("A rest frequency is needed to be able to "
	  "use BasicDataConverter::frequency(MRadialVelocity)");
      }      
      return (*itsVelocityConverter)((*itsDopplerConverter)(in));
    }
    
    
private:
    boost::shared_ptr<IEpochConverter>      itsEpochConverter;
    boost::shared_ptr<IDirectionConverter>  itsDirectionConverter;
    boost::shared_ptr<GenericConverter<casa::MFrequency> >
                                            itsFrequencyConverter;
    boost::shared_ptr<GenericConverter<casa::MRadialVelocity> >
                                            itsVelocityConverter;
    boost::shared_ptr<IDopplerConverter>    itsDopplerConverter;
};
  
} // namespace synthesis

} // namespace conrad
#endif // BASIC_DATA_CONVERTER_H
