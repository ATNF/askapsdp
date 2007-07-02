/// @file
/// @brief Helper functions for dealing with Params for synthesis
///
/// Adds some useful functions specific to synthesis
/// @todo Function to output nicely formatted axes
/// @todo Functions to read/write images
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
        /// @param direction Strings containing [ra, dec, frame]
        /// @param cellsize Cellsize as a string e.g. [12arcsec, 12arcsec]
        /// @param shape Number of pixels in RA and DEC e.g. [256, 256]
        /// @param freqmin Minimum frequency (Hz)
        /// @param freqmax Maximum frequency (Hz)
        /// @param nchan Number of spectral channels
        static void add(conrad::scimath::Params& ip, const string& name, 
          const vector<string>& direction, 
          const vector<string>& cellsize, 
          const vector<int>& shape,
          const double freqmin, const double freqmax, const int nchan);
          
        /// @brief Add a parameter as an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param image Name of image file
        static void add(conrad::scimath::Params& ip, const string& name, 
          const string& image);
          
        /// @brief Save a parameter as an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param image Name of image file
        static void saveAsCasaImage(conrad::scimath::Params& ip, const string& name,
          const string& imagename);
    };

  }
}
#endif
