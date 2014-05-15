/// @file
/// @brief Parset file-based implementation of the calibration solution accessor
/// @details This implementation is to be used with pre-existing code writing/reading
/// the parset directly and with a number of tests. It doesn't implement anything related
/// to bandpass table (and always returns 1. for bandpass and throws an exception if one
/// attempts to write a bandpass). This is because none of the code written so far deals with
/// bandpass tables (and any future code will used in conjunction with more a flexible 
/// implementation, e.g. table-based). This implementation is just to convert the legacy code.
/// There is only one implementation of this class which is used for both reading and writing.
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#include <calibaccess/CachedCalSolutionAccessor.h>
#include <askap/AskapError.h>

namespace askap {

namespace accessors {

/// @brief default constructor
/// @details It initialises a new copy of scimath::Params class to be used as a cache.
CachedCalSolutionAccessor::CachedCalSolutionAccessor() :
   itsCache(new scimath::Params) {}

/// @brief constructor setting up an explicit cache to use 
/// @details It sets up the accessor to use the cache referred to by the given shared 
/// pointer ensuring the reference semantics.
/// @param[in] cache shared pointer to the Params class to use as a cache
CachedCalSolutionAccessor::CachedCalSolutionAccessor(const boost::shared_ptr<scimath::Params> &cache) : itsCache(cache)
{
  ASKAPCHECK(itsCache, "An attempt to initialise CachedCalSolutionAccessor with a void shared pointer");
}        
    
/// @brief copy constructor
/// @details it is required because the actual cache is referred to by the shared pointer. This method
/// clones the cache.
/// @param[in] src a reference to another instance of the class of this type
CachedCalSolutionAccessor::CachedCalSolutionAccessor(const CachedCalSolutionAccessor &src) :
    itsCache(src.cache().clone()) {}
     
// implementation of abstract methods of the interface
  
/// @brief obtain gains (J-Jones)
/// @details This method retrieves parallel-hand gains for both 
/// polarisations (corresponding to XX and YY). If no gains are defined
/// for a particular index, gains of 1. with invalid flags set are
/// returned.
/// @param[in] index ant/beam index 
/// @return JonesJTerm object with gains and validity flags
JonesJTerm CachedCalSolutionAccessor::gain(const JonesIndex &index) const
{
  casa::Complex g1(1.,0.), g2(1.,0.);
  bool g1Valid = false, g2Valid = false;
  const std::string paramG1 = paramName(index, casa::Stokes::XX);
  const std::string paramG2 = paramName(index, casa::Stokes::YY);
  
  if (cache().has(paramG1)) {
      g1Valid = true;
      g1 = cache().complexValue(paramG1);
  }
  if (cache().has(paramG2)) {
      g2Valid = true;
      g2 = cache().complexValue(paramG2);
  }    
  return JonesJTerm(g1,g1Valid,g2,g2Valid);
}
   
/// @brief obtain leakage (D-Jones)
/// @details This method retrieves cross-hand elements of the 
/// Jones matrix (polarisation leakages). There are two values
/// (corresponding to XY and YX) returned (as members of JonesDTerm 
/// class). If no leakages are defined for a particular index,
/// zero leakages are returned with invalid flags set. 
/// @param[in] index ant/beam index
/// @return JonesDTerm object with leakages and validity flags
JonesDTerm CachedCalSolutionAccessor::leakage(const JonesIndex &index) const
{
  casa::Complex d12(0.,0.), d21(0.,0.);
  bool d12Valid = false, d21Valid = false;
  const std::string paramD12 = paramName(index, casa::Stokes::XY);
  const std::string paramD21 = paramName(index, casa::Stokes::YX);
  
  if (cache().has(paramD12)) {
      d12Valid = true;
      d12 = cache().complexValue(paramD12);
  }
  if (cache().has(paramD21)) {
      d21Valid = true;
      d21 = cache().complexValue(paramD21);
  }    
  return JonesDTerm(d12, d12Valid, d21, d21Valid);
}
   
/// @brief obtain bandpass (frequency dependent J-Jones)
/// @details This method retrieves parallel-hand spectral
/// channel-dependent gain (also known as bandpass) for a
/// given channel and antenna/beam. The actual implementation
/// does not necessarily store these channel-dependent gains
/// in an array. It could also implement interpolation or 
/// sample a polynomial fit at the given channel (and 
/// parameters of the polynomial could be in the database). If
/// no bandpass is defined (at all or for this particular channel),
/// gains of 1.0 are returned (with invalid flag is set).
/// @return JonesJTerm object with gains and validity flags
JonesJTerm CachedCalSolutionAccessor::bandpass(const JonesIndex &, const casa::uInt) const
{
  // always return 1.0 as bandpass gain for all spectral channels
  return JonesJTerm(1., true, 1., true);
}

/// @brief helper method to update given parameter in the cache
/// @details Different methods of scimath::Params have to be used depending on whether
/// this parameter is new or not. This method makes it simpler by encapsulating this logic.
/// In addition it handles the logic on what to do with invalid data (for now we just ignore 
/// such values).
/// @param[in] name string name of the parameter
/// @param[in] val complex value to be set
/// @param[in] isValid true, if the given value is valid (method just returns otherwise)
void CachedCalSolutionAccessor::updateParamInCache(const std::string &name, const casa::Complex &val, const bool isValid)
{
  if (isValid) {
      if (cache().has(name)) {
          cache().update(name, val);
      } else {
          cache().add(name, val);
      }
  }
}
  
/// @brief set gains (J-Jones)
/// @details This method writes parallel-hand gains for both 
/// polarisations (corresponding to XX and YY)
/// @param[in] index ant/beam index 
/// @param[in] gains JonesJTerm object with gains and validity flags
void CachedCalSolutionAccessor::setGain(const JonesIndex &index, const JonesJTerm &gains)
{
  updateParamInCache(paramName(index, casa::Stokes::XX), gains.g1(), gains.g1IsValid());
  updateParamInCache(paramName(index, casa::Stokes::YY), gains.g2(), gains.g2IsValid());
}
   
/// @brief set leakages (D-Jones)
/// @details This method writes cross-pol leakages  
/// (corresponding to XY and YX)
/// @param[in] index ant/beam index 
/// @param[in] leakages JonesDTerm object with leakages and validity flags
void CachedCalSolutionAccessor::setLeakage(const JonesIndex &index, const JonesDTerm &leakages)
{
  updateParamInCache(paramName(index, casa::Stokes::XY), leakages.d12(), leakages.d12IsValid());
  updateParamInCache(paramName(index, casa::Stokes::YX), leakages.d21(), leakages.d21IsValid());
}
  
/// @brief set gains for a single bandpass channel
/// @details This method writes parallel-hand gains corresponding to a single
/// spectral channel (i.e. one bandpass element).
/// @param[in] index ant/beam index 
/// @param[in] bp JonesJTerm object with gains for the given channel and validity flags
/// @param[in] chan spectral channel
/// @note We may add later variants of this method assuming that the bandpass is
/// approximated somehow, e.g. by a polynomial. For simplicity, for now we deal with 
/// gains set explicitly for each channel.
void CachedCalSolutionAccessor::setBandpass(const JonesIndex &index, const JonesJTerm &bp, const casa::uInt chan)
{
  ASKAPTHROW(AskapError, "Attempt to set bandpass for ant="<<index.antenna()<<" beam="<<index.beam()<<" chan="<<chan<<
             "(g1="<<bp.g1()<<" g2="<<bp.g2()<<" validity flags: "<<
             bp.g1IsValid()<<","<<bp.g2IsValid()<<"); Operation is not implemented");
}

/// @brief direct access to the cache
/// @return a reference to the cache
/// @note an exception is thrown if the underlying shared pointer is not initialised
scimath::Params& CachedCalSolutionAccessor::cache() const
{
  ASKAPDEBUGASSERT(itsCache);
  return *itsCache;
}


} // namespace accessors

} // namespace askap

