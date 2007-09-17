/// @file
/// 
/// @brief A measurement equation describing antenna gains.
/// @details This measurement equation just multiplies by a gain matrix
/// visibilities produced by another measurement equation. It also generates
/// normal equations, which allow to solve for unknowns in the gain matrix.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef GAIN_CALIBRATION_EQUATION_H
#define GAIN_CALIBRATION_EQUATION_H

// std includes
#include <vector>
#include <utility>

// own includes
#include <fitting/Equation.h>
#include <fitting/Params.h>
#include <fitting/NormalEquations.h>

#include <measurementequation/MultiChunkEquation.h>
#include <dataaccess/IDataAccessor.h>

#include <dataaccess/CachedAccessorField.tcc>


namespace conrad {

namespace synthesis {

/// @brief A measurement equation describing antenna gains.
/// @details This measurement equation just multiplies by a gain matrix
/// visibilities produced by another measurement equation. It also generates
/// normal equations, which allow to solve for unknowns in the gain matrix.
/// @ingroup measurementequation
class GainCalibrationEquation : virtual public MultiChunkEquation,
                                virtual public conrad::scimath::Equation
{
public:

  /// @brief Standard constructor using the parameters and the
  /// data iterator.
  /// @param[in] ip Parameters
  /// @param[in] idi data iterator
  /// @param[in] ime measurement equation describing perfect visibilities
  /// @note In the future, measurement equations will work with accessors
  /// only, and, therefore, the dependency on iterator will be removed
  GainCalibrationEquation(const conrad::scimath::Params& ip,
          const IDataSharedIter& idi, const IMeasurementEquation &ime);
  
  /// @brief Predict model visibilities for one accessor (chunk).
  /// @details This version of the predict method works with
  /// a single chunk of data only. It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). In the future, I expect that
  /// predict() without parameters will be deprecated.
  /// @param[in] chunk a read-write accessor to work with
  virtual void predict(IDataAccessor &chunk) const;

  /// @brief Calculate the normal equation for one accessor (chunk).
  /// @details This version of the method works on a single chunk of
  /// data only (one iteration).It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). In the future, I expect that
  /// the variant of the method without parameters will be deprecated.
  /// @param[in] chunk a read-write accessor to work with
  /// @param[in] ne Normal equations
  virtual void calcEquations(const IConstDataAccessor &chunk,
                                   conrad::scimath::NormalEquations& ne) const;
  
  using MultiChunkEquation::predict;
  using MultiChunkEquation::calcEquations;
protected:
  /// @brief type of the gains cache
  typedef std::vector<std::pair<casa::Complex, casa::Complex> > GainsCacheType;

  // @brief fill the gains cache from parameters
  // @param[in] a reference to the container to fill with values
  void fillGainsCache(GainsCacheType &in) const;
  
  /// @brief helper method to split parameter string
  /// @details Parameters have a form similar to "gain.g11.dt0.25",
  /// one needs to have a way to extract this information from the string.
  /// This method splits a string on each dot symbol and appends the all
  /// substring to a given vector.
  /// @param[in] str input string.
  /// @param[out] parts non-const reference to a vector where substrings will
  ///                   be added to. 
  static void splitParameterString(const std::string &str,
                     std::vector<std::string> &parts) throw(); 
   
private:
  /// @brief measurement equation giving perfect visibilities
  const IMeasurementEquation &itsPerfectVisME;  
  
  
  /// @brief unpacked parameters
  /// @details Currently gains are just antenna dependent. In the future,
  /// we can expect to work with time-, feed- and frequency-dependent cases.
  /// most likely the type of value stored in the container will be changed
  /// to account for various possible situations.
  /// The current meaning of each element is g11 and g22 for each antenna.
  CachedAccessorField<GainsCacheType> itsGainsCache;
  
  /// @brief cache of names of the parameters
  /// @details in the future, we will probably hold this information inside
  /// GainsCacheType
  mutable std::vector<std::string> itsNameCache;
};


/// a number of helper functions are gathered in this namespace
namespace utility {

// probably something like this exists somewhere in the Blob support.
// I grabbed this code from one of my old programs to speed up the development.
// If the appropriate code existing somewhere else in conrad is structured out
// we can move to use it instead. 

/// @brief helper method to interpret string
/// @details any type supported by the input stream can be converted
/// using this method (e.g. string to numbers)
/// @param[in] str input string
/// @return result of the conversion
/// @exception ConradError is thrown if the conversion failed
template<class T> T fromString(const std::string &str) throw(std::bad_cast) {
         std::istringstream is(str);
         T buf;
         is>>buf;
         if (!is) {
             CONRADTHROW(ConradError, "Unable to convert "<<str);
         } 
         return buf;
}

/// @brief helper method to convert any type (e.g. numbers) to a string
/// @details any type supported by the input stream can be converted
/// using this method.
/// @param[in] a const reference to the value to convert
/// @return resulting string
/// @exception ConradError is thrown if the conversion failed
template<class T> std::string toString(const T &in) throw(std::bad_cast) {
         std::ostringstream os;
         os<<in;
         if (!os) {
             CONRADTHROW(ConradError, "Unable to convert "<<in<<" to string");
         }
         return os.str();
}

} // namespace utility


} // namespace synthesis

} // namespace conrad

#endif // #ifndef GAIN_CALIBRATION_EQUATION_H
