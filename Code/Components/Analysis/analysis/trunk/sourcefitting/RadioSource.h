/// @file
///
/// Defines a radio source, combining the Duchamp Detection object
///  with fitted component analysis
///
/// (c) 2008 ASKAP, All Rights Reserved
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef CONRAD_ANALYSIS_RADIOSOURCE_H_
#define CONRAD_ANALYSIS_RADIOSOURCE_H_

#include <duchamp/fitsHeader.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/Detection/detection.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
//#include <casa/iostream.h>

#include <casa/namespace.h>

#include <map>
#include <vector>

namespace conrad
{

  namespace sourcefitting
  {

    /// @brief Width of border to put around detections for fitting purposes, in pixels
    const int detectionBorder = 3;


    /// @brief Class to store all information on a detected source.
    ///
    /// @details This class is designed to hold all appropriate
    /// information on a source detected in an image or cube. A
    /// duchamp::Detection class is used to record the pixel and world
    /// coordinate information, as well as the pixel-based flux information
    /// (peak flux, total flux, etc). However the RadioSource class is
    /// designed to be able to fit an object with known functions (primarily
    /// gaussians) and store the fitted parameters.
    class RadioSource
    {
    public:
      /// @brief Constructor
      RadioSource();
      
      /// @briefCopy constructor for RadioSource.
      //      RadioSource(const RadioSource& r);
      
      /// @brief Assignment operator for RadioSource.
      //RadioSource& operator= (const RadioSource& r);
      
      /// @brief Destructor
      virtual ~RadioSource(){};

      /// @brief Defind the array of fluxes surrounding the Detection.
      bool setFluxArray(std::vector<PixelInfo::Voxel> *voxelList);

      /// @brief Find the local maxima in the flux distribution of the Detection.
      std::multimap<int,PixelInfo::Voxel> RadioSource::findDistinctPeaks();

      /// @brief Fit Gaussian components to the Detection.
      bool fitGauss();

      /// @brief Store a Detection
      void setDetection(duchamp::Detection *object){itsDetection = object;};

      /// @brief Store the FITS header information
      void setHeader(duchamp::FitsHeader *head){itsHeader = head;};

      /// @brief Set the noise level
      void setNoiseLevel(float noise){itsNoiseLevel = noise;};
      
      /// @brief Set the detection threshold
      void setDetectionThreshold(float threshold){itsDetectionThreshold = threshold;};

      /// @brief Print information on the fitted components
      void printFit();
      
    protected:

      /// @brief The object produced by Duchamp-like source-detection.
      duchamp::Detection *itsDetection;

      /// @brief The dimensions of the data structure the object comes from.
      std::vector<int> itsArrayDim;

      /// @brief A pointer to the FITS header information (including WCS and beam info).
      duchamp::FitsHeader *itsHeader;

      /// @brief The noise level in the cube, used for scaling fluxes.
      float itsNoiseLevel;

      /// @brief The detection threshold used for the object
      float itsDetectionThreshold;

      /// @brief A two-dimensional Gaussian fit to the object.
      std::vector<casa::Gaussian2D<Double> > itsGaussFitSet;

      /// @brief The array of flux values for the detection
      float *itsFluxArray;

    };
  }

}



#endif
