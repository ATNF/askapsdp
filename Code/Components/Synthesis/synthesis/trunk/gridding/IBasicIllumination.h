/// @file 
/// @brief Interface to a basic illumination pattern
/// @details This class is an abstract base (i.e. an interface) to 
/// an hierarchy of classes representing illumination patterns.
/// It provides a method to obtain illumination pattern by populating a 
/// pre-defined grid supplied as a UVPattern object. 
///
/// @copyright (c) 2008 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef I_BASIC_ILLUMINATION_H
#define I_BASIC_ILLUMINATION_H

#include <gridding/UVPattern.h>

namespace askap {

namespace synthesis {

/// @brief Interface to a basic illumination pattern
/// @details This class is an abstract base (i.e. an interface) to 
/// an hierarchy of classes representing illumination patterns.
/// It provides a method to obtain illumination pattern by populating a 
/// pre-defined grid supplied as a UVPattern object. 
/// @ingroup gridding
struct IBasicIllumination {
  
  /// @brief obtain illumination pattern
  /// @details This is the main method which populates the 
  /// supplied uv-pattern with the values corresponding to the model
  /// represented by this object. It has to be overridden in the 
  /// derived classes. An optional phase slope can be applied to
  /// simulate offset pointing.
  /// @param[in] freq frequency in Hz for which an illumination pattern is required
  /// @param[in] pattern a UVPattern object to fill
  /// @param[in] l angular offset in the u-direction (in radians)
  /// @param[in] m angular offset in the v-direction (in radians)
  /// @param[in] pa parallactic angle, or strictly speaking the angle between 
  /// uv-coordinate system and the system where the pattern is defined
  virtual void getPattern(double freq, UVPattern &pattern, double l = 0., 
                          double m = 0., double pa = 0.) const = 0;
  
  /// @brief empty virtual destructor to keep the compiler happy
  virtual ~IBasicIllumination();
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef I_BASIC_ILLUMINATION_H
