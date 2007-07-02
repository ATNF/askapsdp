/// @file DopplerConverter.cc
/// @brief A class for interconversion between frequencies
/// and velocities.
/// @details This is an implementation of a relatively low-level
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

/// CASA include
#include <casa/Quanta/MVDoppler.h>
// temporary
#include <casa/Exceptions/Error.h>

/// own includes
#include <dataaccess/DopplerConverter.h>

using namespace conrad;
using namespace conrad::synthesis;


/// constructor
/// @param restFreq The rest frequency used for interconversion between
///                 frequencies and velocities
/// @param velType velocity (doppler) type (i.e. radio, optical)
/// Default is radio definition.
DopplerConverter::DopplerConverter(const casa::MVFrequency &restFreq,
                                   casa::MDoppler::Types velType) :
     itsToBettaConv(velType, casa::MDoppler::BETA),
     itsFromBettaConv(casa::MDoppler::BETA, velType),
     itsRestFrequency(restFreq.getValue()) {}

/// setting the measure frame doesn't make sense for this class
/// because we're not doing conversions here. This method is empty.
/// Defined here to make the compiler happy.
///
/// @param frame  MeasFrame object (can be constructed from
///               MPosition or MEpoch on-the-fly). Not used.
void DopplerConverter::setMeasFrame(const casa::MeasFrame &)
{
}

/// convert specified frequency to velocity in the same reference
/// frame. Velocity definition (i.e. optical or radio, etc) is
/// determined by the implementation class.
///
/// @param freq an MFrequency measure to convert.
/// @return a reference on MRadialVelocity object with the result
const casa::MRadialVelocity&
DopplerConverter::operator()(const casa::MFrequency &freq) const
{  
  casa::Double t=freq.getValue().getValue(); // extract frequency in Hz

  // need to change the next line to a proper Conrad error when available
  CONRADDEBUGASSERT(t!=0);
  
  t/=itsRestFrequency; // form nu/nu_0
  t*=t; // form (nu/nu_0)^2  
  itsRadialVelocity=casa::MRadialVelocity::fromDoppler(itsFromBettaConv(
                 casa::MVDoppler((1.-t)/(1.+t))),
                 freqToVelType(casa::MFrequency::castType(
		               freq.getRef().getType())));
  return itsRadialVelocity;		 
}


/// convert specified velocity to frequency in the same reference
/// frame. Velocity definition (i.e. optical or radio, etc) is
/// determined by the implementation class.
///
/// @param vel an MRadialVelocity measure to convert.
/// @return a reference on MFrequency object with the result
const casa::MFrequency&
DopplerConverter::operator()(const casa::MRadialVelocity &vel) const
{
  itsFrequency=casa::MFrequency::fromDoppler(
        itsToBettaConv(casa::MVDoppler(vel.getValue().get())),
	               casa::MVFrequency(itsRestFrequency),
		       velToFreqType(casa::MRadialVelocity::castType(
		                    vel.getRef().getType())));
  return itsFrequency;		       
}

/// convert frequency frame type to velocity frame type
/// @param type frequency frame type to convert
/// @return resulting velocity frame type
///
/// Note, an exception is thrown if the the frame type is
/// MFrequency::REST (it doesn't make sense to always return zero
/// velocity).
casa::MRadialVelocity::Types
DopplerConverter::freqToVelType(casa::MFrequency::Types type)
                                throw(DataAccessLogicError)
{
  switch(type) {
    case casa::MFrequency::LSRK: return casa::MRadialVelocity::LSRK;
    case casa::MFrequency::LSRD: return casa::MRadialVelocity::LSRD;
    case casa::MFrequency::BARY: return casa::MRadialVelocity::BARY;
    case casa::MFrequency::GEO: return casa::MRadialVelocity::GEO;
    case casa::MFrequency::TOPO: return casa::MRadialVelocity::TOPO;
    case casa::MFrequency::GALACTO: return casa::MRadialVelocity::GALACTO;
    case casa::MFrequency::LGROUP: return casa::MRadialVelocity::LGROUP;
    case casa::MFrequency::CMB: return casa::MRadialVelocity::CMB;
    default: throw DataAccessLogicError("DopplerConverter: Unable to convert "
                              "freqency frame type to velocity frame type");
  };

  // to keep the compiler happy. It should never go this far.
  return casa::MRadialVelocity::LSRK; 
}

/// convert velocity frame type to frequency frame type
/// @param type velocity frame type to convert
/// @return resulting frequency frame type
casa::MFrequency::Types
DopplerConverter::velToFreqType(casa::MRadialVelocity::Types type)
                                             throw(DataAccessLogicError)
{
  switch(type) {
    case casa::MRadialVelocity::LSRK: return casa::MFrequency::LSRK;
    case casa::MRadialVelocity::LSRD: return casa::MFrequency::LSRD;
    case casa::MRadialVelocity::BARY: return casa::MFrequency::BARY;
    case casa::MRadialVelocity::GEO: return casa::MFrequency::GEO;
    case casa::MRadialVelocity::TOPO: return casa::MFrequency::TOPO;
    case casa::MRadialVelocity::GALACTO: return casa::MFrequency::GALACTO;
    case casa::MRadialVelocity::LGROUP: return casa::MFrequency::LGROUP;
    case casa::MRadialVelocity::CMB: return casa::MFrequency::CMB;
    default: throw DataAccessLogicError("DopplerConverter: Unable to convert "
                             "velocity frame type to frequency frame type");
  };

  // to keep the compiler happy. It should never go this far.
  return casa::MFrequency::LSRK;
}
