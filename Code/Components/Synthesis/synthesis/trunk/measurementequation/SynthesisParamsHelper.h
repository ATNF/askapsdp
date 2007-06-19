/// @file
/// @brief Helper functions for dealing with Params for synthesis
///
/// Adds some useful functions specific to synthesis
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNSYNTHESISPARAMSHELPER_H_
#define SYNSYNTHESISPARAMSHELPER_H_

#include <fitting/Params.h>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Helper functions for synthesis processing using Params
    ///
    class SynthesisParamsHelper 
    {
      public:
        /// @brief Add a parameter as an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param ra Right Ascencion
        /// @param dec Declination
        /// @param cellsize Cellsize
        /// @param nx Number of pixels in RA
        /// @param ny Number of pixels in Dec
        /// @param freqmin Minimum frequency (Hz)
        /// @param freqmax Maximum frequency (Hz)
        /// @param nchan Number of spectral channels
        static void add(conrad::scimath::Params& ip, const string& name, 
          const double ra, const double dec, const double cellsize,
          const int nx, const int ny, 
          const double freqmin, const double freqmax, const int nchan);

        /// @brief Add a parameter as an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param image Name of image file
        static void add(conrad::scimath::Params& ip, const string& name, 
          const string& image);
    };

  }
}
#endif
