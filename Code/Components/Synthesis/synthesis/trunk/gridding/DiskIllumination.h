/// @file 
/// @brief Simple disk illumination model
/// @details This class represents a simple illumination model, 
/// which is just a disk of a certain radius with a hole in the centre.
/// Optionally a phase slope can be applied to simulate offset pointing.
///
/// @copyright (c) 2008 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef DISK_ILLUMINATION_H
#define DISK_ILLUMINATION_H

#include <gridding/IBasicIllumination.h>

namespace askap {

namespace synthesis {

/// @brief Simple disk illumination model
/// @details This class represents a simple illumination model, 
/// which is just a disk of a certain radius with a hole in the centre
/// Optionally a phase slope can be applied to simulate offset pointing.
/// @ingroup gridding
struct DiskIllumination : virtual public IBasicIllumination {

  /// @brief construct the model
  /// @param[in] diam disk diameter in metres
  /// @param[in] blockage a diameter of the central hole in metres
  DiskIllumination(double diam, double blockage);
    
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
  virtual void getPattern(double freq, UVPattern &pattern, double l = 0., 
                          double m = 0.) const;
private:
  /// @brief disk diameter in metres
  double itsDiameter;
  
  /// @brief diameter of the central hole in metres
  double itsBlockage;
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef DISK_ILLUMINATION_H

