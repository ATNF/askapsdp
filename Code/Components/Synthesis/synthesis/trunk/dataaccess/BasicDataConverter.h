/// @file BasicDataConverter.h
/// @brief An implementation of the data converter
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///


#ifndef BASIC_DATA_CONVERTER_H
#define BASIC_DATA_CONVERTER_H

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
#include <dataaccess/IDataConverterImpl.h>

namespace askap {

namespace synthesis {

/// @brief
/// An implementation of the data converter (IDataConverter interface).
/// @details
/// The intention is to use this class in conjunction
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
/// @ingroup dataaccess_conv
class BasicDataConverter : virtual public IDataConverterImpl
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
    /// @param[in] origin a zero-point for the visibility timestamps 
    ///        (they are given as time offsets with respect to 
    ///        this origin). A reference frame of this measure is
    ///        used in all time epochs (e.g. selection)
    /// @param[in] unit a required time unit for timestamps
    ///
    /// Class defaults to MJD 0 UTC, timestamp in seconds
    virtual void setEpochFrame(const casa::MEpoch &origin = casa::MEpoch(),
    	   const casa::Unit &unit = "s");
    
    /// set the reference frame for directions. At this moment we
    /// have only pointing direction accessible via DataAccessor.
    /// In the future, selection based on the direction observed can
    /// be added.
    /// @param[in] ref a reference frame to be used for all directions
    ///            (default is J2000).
    /// @param[in] unit units for all direction offsets. Unused at the
    ///             moment. Default units are radians.
    virtual void setDirectionFrame(const casa::MDirection::Ref &ref,
                   const casa::Unit &unit = "rad");
    
    /// set the reference frame for any frequency
    /// (e.g. in the frequency-based selection or frequency to channel
    ///  mapping)
    /// @param[in] ref a reference frame to be used with all frequencies
    /// @param[in] unit frequency units to use (frequencies will be returned
    ///             as Doubles)
    ///
    /// Class defaults to LSRK, GHz
    virtual void setFrequencyFrame(const casa::MFrequency::Ref &ref,
	   const casa::Unit &unit = "GHz");

    /// set the reference frame for any velocity
    /// (e.g. in the velocity-based selection or spectral labelling)
    /// @param[in] ref a reference frame to be used with all velocities
    /// @param[in] unit velocity units to use (velocities will be returned
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
    /// @param[in] restFreq a rest frequency to be used for interconversions
    ///                 between frequencies and velocities
    ///
    virtual void setRestFrequency(const casa::MVFrequency &restFreq);

    /// set a frame (for epochs it is just a position), where the
    /// conversion is performed
    /// @param[in] frame measure's frame object
    virtual void setMeasFrame(const casa::MeasFrame &frame);


    /// following methods are used within the DataSource/DataIterator
    
    /// convert epochs
    /// @param[in] in input epoch given as an MEpoch object
    /// @return epoch converted to Double 
    virtual casa::Double epoch(const casa::MEpoch &in) const;    

    /// reverse conversion: form a measure from 'double' epoch
    /// @param[in] in epoch given as Double in the target units/frame
    /// @return epoch converted to Measure
    virtual casa::MEpoch epochMeasure(casa::Double in) const;

    /// reverse conversion: form a measure from MVEpoch
    /// @param[in] in epoch given as MVEpoch in the target frame
    /// @return epoch converted to Measure
    virtual casa::MEpoch epochMeasure(const casa::MVEpoch &in) const;

    /// convert directions
    /// @param[in] in input direction given as an MDirection object
    /// @param[out] out output direction as an MVDirection object
    virtual void direction(const casa::MDirection &in, 
                          casa::MVDirection &out) const;

    /// test whether the frequency conversion is void
    /// @param[in] testRef reference frame to test
    /// @param[in] testUnit units to test
    virtual bool isVoid(const casa::MFrequency::Ref &testRef,
                        const casa::Unit &testUnit) const;

    /// convert frequencies
    /// @param[in] in input frequency given as an MFrequency object
    /// @return output frequency as a Double
    virtual casa::Double frequency(const casa::MFrequency &in) const;

    /// convert velocities
    /// @param[in] in input velocities given as an MRadialVelocity object
    /// @return output velocity as a Double
    virtual casa::Double velocity(const casa::MRadialVelocity &in) const;    

    /// convert frequencies from velocities
    /// @param[in] in input velocity given as an MRadialVelocity object
    /// @return output frequency as a Double
    ///
    /// @note An exception will be thrown if the rest frequency is not
    /// defined.
    ///
    virtual casa::Double frequency(const casa::MRadialVelocity &in) const;
    

    /// convert velocities from frequencies
    /// @param[in] in input frequency  given as an MFrequency object
    /// @return output velocity as a Double
    ///
    /// @note An exception will be thrown if the rest frequency is not
    /// defined.
    ///
    virtual casa::Double velocity(const casa::MFrequency &in) const;    
    
    /// @brief Clone the converter (sort of a virtual constructor)
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
    virtual boost::shared_ptr<IDataConverterImpl> clone() const;  
    
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

} // namespace askap
#endif // BASIC_DATA_CONVERTER_H
