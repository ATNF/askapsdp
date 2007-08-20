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

#include <APS/ParameterSet.h>
#include <images/Images/TempImage.h>
#include <images/Images/ImageInterface.h>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Helper functions for synthesis processing using Params
    /// @ingroup measurementequation
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
          
        /// @brief Add a set of parameters from a parset
        /// @param ip Parameters
        /// @param parset ParameterSet
        /// @param baseKey basekey for parameters e.g. "Images."
        static void add(conrad::scimath::Params& ip,
          const LOFAR::ACC::APS::ParameterSet& parset,
          const std::string& baseKey);
          
        /// @brief Add a parameter as an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param image Name of image file
        static void add(conrad::scimath::Params& ip, const string& name, 
          const string& image);
          
        /// @brief Get a parameter from a CASA image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param imagename Name of image file
        static void getFromCasaImage(conrad::scimath::Params& ip, const string& name,
          const string& imagename);
        
        /// @brief Save a parameter as a CASA image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param imagename Name of image file
        static void saveAsCasaImage(const conrad::scimath::Params& ip, const string& name,
          const string& imagename);
        
        /// @brief Copy a parameter to a CASA TempImage
        /// Note that this will be a reference if possible
        /// @param ip Parameters
        /// @param name Name of parameter
        static boost::shared_ptr<casa::TempImage<float> > 
          tempImage(const conrad::scimath::Params& ip, 
          const string& name);
       
        /// @brief Create a coordinate system for a parameter
        /// @param ip Parameters
        /// @param name Name of parameter
        static casa::CoordinateSystem 
          coordinateSystem(const conrad::scimath::Params& ip, 
          const string& name);
       
        /// @brief Create a direction coordinate for a parameter
        /// @param ip Parameters
        /// @param name Name of parameter
        static casa::DirectionCoordinate 
          directionCoordinate(const conrad::scimath::Params& ip, 
          const string& name);
       
        /// @brief Update a parameter from an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param image Image to be drawn from 
        static void update(conrad::scimath::Params& ip, const string& name, 
          const casa::ImageInterface<float>& image);
        
    };

  }
}
#endif
