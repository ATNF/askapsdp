/// @file
/// @brief implementation of the calibration solution accessor returning cached values
/// @details This class wraps scimath::Params into a new interface and acts as an adapter.
/// Using this adapter one can achieve a greater reuse of the code in the measurement 
/// equation classes: calibration involves running solver which would benefit from a 
/// direct access to scimath::Params class, in contrast calibration application during imaging
/// could use solution accessor interface.
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

#ifndef CACHED_CAL_SOLUTION_ACCESSOR_H
#define CACHED_CAL_SOLUTION_ACCESSOR_H

// own includes
#include <calibaccess/ICalSolutionAccessor.h>
#include <calibaccess/CalParamNameHelper.h>
#include <fitting/Params.h>

// std includes
#include <string>

namespace askap {

namespace accessors {

/// @brief implementation of the calibration solution accessor returning cached values
/// @details This class wraps scimath::Params into a new interface and acts as an adapter.
/// Using this adapter one can achieve a greater reuse of the code in the measurement 
/// equation classes: calibration involves running solver which would benefit from a 
/// direct access to scimath::Params class, in contrast calibration application during imaging
/// could use solution accessor interface.
/// @ingroup calibaccess
class CachedCalSolutionAccessor : virtual public ICalSolutionAccessor,
                                  virtual protected CalParamNameHelper {
public:
  /// @brief default constructor
  /// @details It initialises a new copy of scimath::Params class to be used as a cache.
  CachedCalSolutionAccessor();

  /// @brief constructor setting up an explicit cache to use 
  /// @details It sets up the accessor to use the cache referred to by the given shared 
  /// pointer ensuring the reference semantics.
  /// @param[in] cache shared pointer to the Params class to use as a cache
  CachedCalSolutionAccessor(const boost::shared_ptr<scimath::Params> &cache);
    
  /// @brief copy constructor
  /// @details it is required because the actual cache is referred to by the shared pointer. This method
  /// clones the cache.
  /// @param[in] src a reference to another instance of the class of this type
  CachedCalSolutionAccessor(const CachedCalSolutionAccessor &src);
    
  // implementation of the abstract methods of the interface
  
  /// @brief obtain gains (J-Jones)
  /// @details This method retrieves parallel-hand gains for both 
  /// polarisations (corresponding to XX and YY). If no gains are defined
  /// for a particular index, gains of 1. with invalid flags set are
  /// returned.
  /// @param[in] index ant/beam index 
  /// @return JonesJTerm object with gains and validity flags
  virtual JonesJTerm gain(const JonesIndex &index) const;
   
  /// @brief obtain leakage (D-Jones)
  /// @details This method retrieves cross-hand elements of the 
  /// Jones matrix (polarisation leakages). There are two values
  /// (corresponding to XY and YX) returned (as members of JonesDTerm 
  /// class). If no leakages are defined for a particular index,
  /// zero leakages are returned with invalid flags set. 
  /// @param[in] index ant/beam index
  /// @return JonesDTerm object with leakages and validity flags
  virtual JonesDTerm leakage(const JonesIndex &index) const;
   
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
  /// @param[in] index ant/beam index
  /// @param[in] chan spectral channel of interest
  /// @return JonesJTerm object with gains and validity flags
  virtual JonesJTerm bandpass(const JonesIndex &index, const casa::uInt chan) const;
  
  /// @brief set gains (J-Jones)
  /// @details This method writes parallel-hand gains for both 
  /// polarisations (corresponding to XX and YY)
  /// @param[in] index ant/beam index 
  /// @param[in] gains JonesJTerm object with gains and validity flags
  virtual void setGain(const JonesIndex &index, const JonesJTerm &gains);
   
  /// @brief set leakages (D-Jones)
  /// @details This method writes cross-pol leakages  
  /// (corresponding to XY and YX)
  /// @param[in] index ant/beam index 
  /// @param[in] leakages JonesDTerm object with leakages and validity flags
  virtual void setLeakage(const JonesIndex &index, const JonesDTerm &leakages);
   
  /// @brief set gains for a single bandpass channel
  /// @details This method writes parallel-hand gains corresponding to a single
  /// spectral channel (i.e. one bandpass element).
  /// @param[in] index ant/beam index 
  /// @param[in] bp JonesJTerm object with gains for the given channel and validity flags
  /// @param[in] chan spectral channel
  /// @note We may add later variants of this method assuming that the bandpass is
  /// approximated somehow, e.g. by a polynomial. For simplicity, for now we deal with 
  /// gains set explicitly for each channel.
  virtual void setBandpass(const JonesIndex &index, const JonesJTerm &bp, const casa::uInt chan);
  
  
  /// @brief direct access to the cache
  /// @return a reference to the cache
  /// @note an exception is thrown if the underlying shared pointer is not initialised
  scimath::Params& cache() const;
  
protected:
  
  /// @brief helper method to update given parameter in the cache
  /// @details Different methods of scimath::Params have to be used depending on whether
  /// this parameter is new or not. This method makes it simpler by encapsulating this logic.
  /// In addition it handles the logic on what to do with invalid data (for now we just ignore 
  /// such values).
  /// @param[in] name string name of the parameter
  /// @param[in] val complex value to be set
  /// @param[in] isValid true, if the given value is valid (method just returns otherwise)
  void updateParamInCache(const std::string &name, const casa::Complex &val, const bool isValid = true);
    
private:  
  /// @brief shared pointer to the cache of parameters
  boost::shared_ptr<scimath::Params> itsCache;
};


} // namespace accessors

} // namespace askap

#endif // #ifndef CACHED_CAL_SOLUTION_ACCESSOR_H

