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

#include <analysisutilities/BlobRelated.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Detection/columns.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian2D.h>

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

	/// @brief Copy constructor for RadioSource.
	RadioSource(const RadioSource& r);
      
	/// @brief Assignment operator for RadioSource.
	RadioSource& operator= (const RadioSource& r);
      
	/// @brief Destructor
	virtual ~RadioSource(){};

	/// @brief Find the local maxima in the flux distribution of the Detection.
	std::multimap<int,PixelInfo::Voxel> findDistinctPeaks(casa::Vector<casa::Double> f);

	//@{
	/// @brief Fit Gaussian components to the Detection.
	bool fitGauss(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
		      casa::Vector<casa::Double> sigma);
	bool fitGauss(std::vector<PixelInfo::Voxel> *voxelList);
	bool fitGauss(float *fluxArray, long *dimArray);
	//@}

	/// @brief Store the FITS header information
	void setHeader(duchamp::FitsHeader head){itsHeader = head;};

	/// @brief Get the FITS header information
	duchamp::FitsHeader getHeader(){return itsHeader;};

	/// @brief Set the noise level
	void setNoiseLevel(float noise){itsNoiseLevel = noise;};
      
	/// @brief Set the detection threshold
	void setDetectionThreshold(float threshold){itsDetectionThreshold = threshold;};

	/// @brief Print information on the fitted components
	void printFit();
      
	/// @brief Return the set of fits
	std::vector<casa::Gaussian2D<Double> > gaussFitSet(){return itsGaussFitSet;};

	/// @brief Print summary of detection & fit
	void printSummary(std::ostream &stream, std::vector<duchamp::Column::Col> columns,
			  bool doHeader);

	/// @brief Write the description of the fits to an annotation file.
	void writeFitToAnnotationFile(std::ostream &stream);

	/// @brief Functions allowing RadioSource objects to be passed over LOFAR Blobs
	// @{
	friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &stream, RadioSource& src);
	friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &stream, RadioSource& src);
	// @}

	/// @brief Is the object at the edge of a subimage
	bool isAtEdge(){return atEdge;};
	/// @brief Set the atEdge flag.
	void setAtEdge(bool b){atEdge = b;};

	/// @brief Commands to return the extent and size of the box
	/// surrounding the object. Uses the detectionBorder parameter.
	/// @name 
	// @{
	/// Minimum x-value
	long boxXmin(){return getXmin() - detectionBorder;};
	/// Maximum x-value
	long boxXmax(){return getXmax() + detectionBorder;};
	/// Minimum y-value
	long boxYmin(){return getYmin() - detectionBorder;};
	/// Maximum y-value
	long boxYmax(){return getYmax() + detectionBorder;};
	/// Minimum z-value
	long boxZmin(){return getZmin() - detectionBorder;};
	/// Maximum z-value
	long boxZmax(){return getZmax() + detectionBorder;};
	/// X-width
	long boxXsize(){return boxXmax()-boxXmin()+1;};
	/// Y-width
	long boxYsize(){return boxYmax()-boxYmin()+1;};
	/// Number of pixels in box
	long boxSize(){return boxXsize()*boxYsize();};
	// @}

	/// @brief Comparison operator, using the name field
	friend bool operator< (RadioSource lhs, RadioSource rhs)
	{
	  if(lhs.getZcentre()==rhs.getZcentre()) return (lhs.name<rhs.name);
	  else return (lhs.getZcentre()<rhs.getZcentre());
	}

	/// @brief Return a reference to the set of Gaussian fits.
	std::vector<casa::Gaussian2D<Double> >& fitset(){
	  std::vector<casa::Gaussian2D<Double> >& rfit = itsGaussFitSet; return rfit;};

      protected:

	/// @brief A flag indicating whether the source is on the boundary of a subimage.
	bool atEdge;

	/// @brief A flag indicating whether a fit has been made to the source.
	bool hasFit;
	
	/// @brief The FITS header information (including WCS and beam info).
	duchamp::FitsHeader itsHeader;

	/// @brief The noise level in the cube, used for scaling fluxes.
	float itsNoiseLevel;

	/// @brief The detection threshold used for the object
	float itsDetectionThreshold;

	/// @brief A two-dimensional Gaussian fit to the object.
	std::vector<casa::Gaussian2D<Double> > itsGaussFitSet;

      };
    }

  }

}



#endif
