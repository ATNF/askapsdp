/// @file
///
/// DopplerConverter: Class for interconversion between frequencies
/// and velocities. This is an implementation of a relatively low-level
/// interface, which is used within the implementation of the data
//// accessor. The end user interacts with the IDataConverter class only.
///
/// The idea behind this class is very similar to CASA's VelocityMachine,
/// but we require a bit different interface to use the class efficiently
/// (and the interface conversion would be equivalent in complexity to
/// the transformation itself). Hence, we will use this class 
/// instead of the VelocityMachine
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// std includes
#include <stdexcept>

/// CASA includes
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVFrequency.h>

/// own includes
#include <dataaccess/IDopplerConverter.h>

#ifndef DOPPLER_CONVERTER_H
#define DOPPLER_CONVERTER_H

namespace conrad {

namespace synthesis {

struct DopplerConverter : virtual public IDopplerConverter {

    /// constructor
    /// @param restFreq The rest frequency used for interconversion between
    ///                 frequencies and velocities
    /// @param velType velocity (doppler) type (i.e. radio, optical)
    /// Default is radio definition.
    explicit DopplerConverter(const casa::MVFrequency &restFreq,
                              casa::MDoppler::Types velType =
                              casa::MDoppler::RADIO);
    
    /// convert specified frequency to velocity in the same reference
    /// frame. Velocity definition (i.e. optical or radio, etc) is
    /// determined by the implementation class.
    ///
    /// @param freq an MFrequency measure to convert.
    /// @return a reference on MRadialVelocity object with the result
    virtual const casa::MRadialVelocity& operator()(const casa::MFrequency &freq) const;

    /// convert specified velocity to frequency in the same reference
    /// frame. Velocity definition (i.e. optical or radio, etc) is
    /// determined by the implementation class.
    ///
    /// @param vel an MRadialVelocity measure to convert.
    /// @return a reference on MFrequency object with the result
    virtual const casa::MFrequency& operator()(const casa::MRadialVelocity &vel) const;
protected:
    /// setting the measure frame doesn't make sense for this class
    /// because we're not doing conversions here. This method is empty.
    /// Defined here to make the compiler happy.
    ///
    /// @param frame  MeasFrame object (can be constructed from
    ///               MPosition or MEpoch on-the-fly). Not used.
    virtual void setMeasFrame(const casa::MeasFrame &);

    /// convert frequency frame type to velocity frame type
    /// @param type frequency frame type to convert
    /// @return resulting velocity frame type
    ///
    /// Note, an exception is thrown if the the frame type is
    /// MFrequency::REST (it doesn't make sense to always return zero
    /// velocity).
    static casa::MRadialVelocity::Types
        freqToVelType(casa::MFrequency::Types type) throw(std::logic_error);

    /// convert velocity frame type to frequency frame type
    /// @param type velocity frame type to convert
    /// @return resulting frequency frame type
    static casa::MFrequency::Types
      velToFreqType(casa::MRadialVelocity::Types type) throw(std::logic_error);
		 
private:
    /// doppler converters:
    /// from own velocity type specified in the constructor
    ///   to BETA (true velocity)
    mutable casa::MDoppler::Convert itsToBettaConv;
    /// from true velocity to velocity type specified in the constructor
    mutable casa::MDoppler::Convert itsFromBettaConv;

    /// rest frequency required for conversion in Hz
    casa::Double itsRestFrequency;

    /// result buffers
    mutable casa::MRadialVelocity itsRadialVelocity;
    mutable casa::MFrequency itsFrequency;
};

} // namespace synthesis

} // namespace conrad



#endif // #ifndef DOPPLER_CONVERTER_H
