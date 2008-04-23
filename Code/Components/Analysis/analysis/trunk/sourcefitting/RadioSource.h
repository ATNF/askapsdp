/// @file
///
/// Defines a radio source, combining the Duchamp Detection object
///  with fitted component analysis
///
/// (c) 2008 ASKAP, All Rights Reserved
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_RADIOSOURCE_H_
#define ASKAP_ANALYSIS_RADIOSOURCE_H_

#include <duchamp/fitsHeader.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Detection/columns.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
//#include <casa/iostream.h>

#include <casa/namespace.h>

#include <map>
#include <vector>

//using namespace duchamp;

namespace askap
{

  namespace analysis
  {

    /// @ingroup sourcefitting
    namespace sourcefitting
    {

      /// @brief Width of border to put around detections for fitting purposes, in pixels
      const int detectionBorder = 3;


      /// @brief Class to store all information on a detected source.
      ///
      /// @details This class is designed to hold all appropriate
      /// information on a source detected in an image or cube. It derives from the
      /// duchamp::Detection class, and so records the pixel and world
      /// coordinate information, as well as the pixel-based flux information
      /// (peak flux, total flux, etc). However the RadioSource class is
      /// designed to be able to fit an object with known functions (primarily
      /// gaussians) and store the fitted parameters.
      ///
      /// @ingroup sourcefitting
      class RadioSource : public duchamp::Detection
      {
      public:
	/// @brief Constructor
	RadioSource();

	/// @brief Constructor using information in a duchamp::Detection object.
	RadioSource(duchamp::Detection obj);

	/// @briefCopy constructor for RadioSource.
	//      RadioSource(const RadioSource& r);
      
	/// @brief Assignment operator for RadioSource.
	//RadioSource& operator= (const RadioSource& r);
      
	/// @brief Destructor
	virtual ~RadioSource(){};

	/// @brief Defind the array of fluxes surrounding the Detection.
	bool setFluxArray(std::vector<PixelInfo::Voxel> *voxelList);

	/// @brief Find the local maxima in the flux distribution of the Detection.
	std::multimap<int,PixelInfo::Voxel> findDistinctPeaks();

	/// @brief Fit Gaussian components to the Detection.
	bool fitGauss();

	/// @brief Store the FITS header information
	void setHeader(duchamp::FitsHeader *head){itsHeader = head;};

	/// @brief Set the noise level
	void setNoiseLevel(float noise){itsNoiseLevel = noise;};
      
	/// @brief Set the detection threshold
	void setDetectionThreshold(float threshold){itsDetectionThreshold = threshold;};

	/// @brief Print information on the fitted components
	void printFit();
      
	/// @brief Return the set of fits
	std::vector<casa::Gaussian2D<Double> > gaussFitSet(){return itsGaussFitSet;};

	/// @brief Print summary of detection & fit
	void printSummary(std::ostream &stream, std::vector<duchamp::Column::Col> columns);

	/// @brief Write the description of the fits to an annotation file.
	void writeFitToAnnotationFile(std::ostream &stream);
      
      protected:

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

}



#endif
