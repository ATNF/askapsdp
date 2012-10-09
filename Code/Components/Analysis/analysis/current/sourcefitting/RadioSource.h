/// @file
///
/// Defines a radio source, combining the Duchamp Detection object
///  with fitted component analysis
///
/// @copyright (c) 2008 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#ifndef ASKAP_ANALYSIS_RADIOSOURCE_H_
#define ASKAP_ANALYSIS_RADIOSOURCE_H_

#include <sourcefitting/FittingParameters.h>
#include <sourcefitting/FitResults.h>
#include <sourcefitting/Component.h>
#include <analysisutilities/SubimageDef.h>

#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/Detection/detection.hh>
/* #include <duchamp/Detection/columns.hh> */
#include <duchamp/Outputs/columns.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Section.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian2D.h>

#include <casa/namespace.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>

#include <map>
#include <vector>
#include <utility>
#include <string>

//using namespace duchamp;

namespace askap {

    namespace analysis {

        namespace sourcefitting {

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
            class RadioSource : public duchamp::Detection {
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
                    virtual ~RadioSource() {};

                    /// @brief Find the local maxima in the flux distribution of the Detection.
                    std::multimap<int, PixelInfo::Voxel> findDistinctPeaks(casa::Vector<casa::Double> f);

                    /// @brief Estimate the FWHM of the Detection.
                    void getFWHMestimate(float *fluxarray, double &angle, double &maj, double &min);

                    /// @brief Return a list of subcomponents.
                    std::vector<SubComponent> getSubComponentList(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> &f);
                    /// @brief Return a list of subcomponents that lie above a flux threshold
                    std::vector<SubComponent> getThresholdedSubComponentList(float *fluxarray);

                    /// @brief Fit Gaussian components to the Detection.
                    /// @name
                    ///@{
                    bool fitGaussNew(std::vector<PixelInfo::Voxel> *voxelList, FittingParameters &baseFitter);
                    bool fitGauss(std::vector<PixelInfo::Voxel> *voxelList, FittingParameters &baseFitter);
/*D1.1.13           bool fitGauss(float *fluxArray, long *dimArray, FittingParameters &baseFitter); */
                    bool fitGauss(float *fluxArray, size_t *dimArray, FittingParameters &baseFitter);
                    bool fitGauss(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
                                  casa::Vector<casa::Double> sigma, FittingParameters &baseFitter);
                    ///@}

                    /// @brief Store the FITS header information
                    void setHeader(duchamp::FitsHeader head) {itsHeader = head;};

                    /// @brief Get the FITS header information
                    duchamp::FitsHeader getHeader() {return itsHeader;};

                    /// @brief Functions to set the object's noise level
                    /// @name
                    /// @{

                    /// @brief Set the noise level to the local value, using an array.
/*D1.1.13           void setNoiseLevel(float *array, long *dim, int boxSize = defaultNoiseBoxSize); */
                    void setNoiseLevel(float *array, size_t *dim, int boxSize = defaultNoiseBoxSize);

                    /// @brief Set the noise level to the local value, using a Duchamp::Cube object
                    void setNoiseLevel(duchamp::Cube &cube, FittingParameters &fitparams);

                    /// @brief Set the noise level
                    void setNoiseLevel(float noise) {itsNoiseLevel = noise;};
                    /// @}

		    /// @brief Set the detection threshold for a particular Cube
		    void setDetectionThreshold(duchamp::Cube &cube, bool flagMedianSearch);
                    /// @brief Set the detection threshold directly
                    void setDetectionThreshold(float threshold) {itsDetectionThreshold = threshold;};
		    void setDetectionThreshold(std::vector<PixelInfo::Voxel> &inVoxlist,std::vector<PixelInfo::Voxel> &inSNRvoxlist,  bool flagMedianSearch);
                    /// @brief Return the detection threshold
                    float detectionThreshold() {return itsDetectionThreshold;};

                    /// @brief Return the set of fits
                    std::vector<casa::Gaussian2D<Double> > gaussFitSet(std::string type) {return itsBestFitMap[type].fitSet();};
                    std::vector<casa::Gaussian2D<Double> > gaussFitSet() {return itsBestFitMap["best"].fitSet();};
		    
		    /// @brief Return the number of fits for a fit type
		    int numFits(std::string type) {return itsBestFitMap[type].numFits();};
		    /// @brief Return the number of fits for the best set
		    int numFits() {return itsBestFitMap["best"].numFits();};
		    
                    /// @brief Return a reference to the set of Gaussian fits.
                    std::vector<casa::Gaussian2D<Double> >& fitset(std::string type) {return itsBestFitMap[type].fits();};

                    /// @brief Print summary of detection & fit
                    /* void printSummary(std::ostream &stream, std::vector<duchamp::Column::Col> columns, */
                    /*                   std::string fittype, bool doHeader = false); */
		    void printSummary(std::ostream &stream, duchamp::Catalogues::CatalogueSpecification columns,
                                      std::string fittype, bool doHeader = false);

                    /// @brief Write the description of the fits to an annotation file.
                    void writeFitToAnnotationFile(std::ostream &stream, bool doEllipse, bool doBox);

                    /// @brief Functions allowing RadioSource objects to be passed over LOFAR Blobs
                    /// @name
                    /// @{

                    /// @brief Pass a RadioSource object into a Blob
                    friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &stream, RadioSource& src);
                    /// @brief Receive a RadioSource object from a Blob
                    friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &stream, RadioSource& src);

                    /// @}

                    /// @brief Is the object at the edge of a subimage
                    bool isAtEdge() {return atEdge;};
                    /// @brief Set the atEdge flag.
                    void setAtEdge(bool b) {atEdge = b;};
                    /// @brief Set the atEdge flag using information from a Cube
                    void setAtEdge(duchamp::Cube &cube, SubimageDef &subimage, int workerNum);

                    /// @brief Define the boundaries of the box
                    void defineBox(duchamp::Section &sec, FittingParameters &fitParams, int spectralAxis = 2);

                    /// @brief Commands to return the extent and size of the box
                    /// surrounding the object.
                    /// @name
                    // @{

                    /// Minimum x-value
                    long boxXmin() {return itsBox.start()[0];};
                    /// Maximum x-value
                    long boxXmax() {return itsBox.end()[0];};
                    /// Minimum y-value
                    long boxYmin() {return itsBox.start()[1];};
                    /// Maximum y-value
                    long boxYmax() {return itsBox.end()[1];};
                    /// Minimum z-value
                    long boxZmin() {return itsBox.start()[2];};
                    /// Maximum z-value
                    long boxZmax() {return itsBox.end()[2];};
                    /// X-width
                    long boxXsize() {return itsBox.length()[0];};
                    /// Y-width
                    long boxYsize() {return itsBox.length()[1];};
                    /// Number of pixels in box
                    long boxSize() {return boxXsize()*boxYsize();};

                    /// Return the full box description
                    casa::Slicer box() {return itsBox;};
                    /// Define the box in one shot
                    void setBox(casa::Slicer box) {itsBox = box;};

                    // @}

                    /// @brief Comparison operator, using the name field
                    friend bool operator< (RadioSource lhs, RadioSource rhs) {
                        if (lhs.getZcentre() == rhs.getZcentre()) return (lhs.name < rhs.name);
                        else return (lhs.getZcentre() < rhs.getZcentre());
                    }

                    /// @brief Return a reference to the fitting parameters
                    FittingParameters &fitparams() {FittingParameters& rfitpars = itsFitParams; return rfitpars;};

                    /// @brief Set the fitting parameters by passing a set
                    void setFitParams(FittingParameters &fitpars) {itsFitParams = fitpars;};

		    /// @brief Find the spectral index or curvature for each fitted component in the source
		    void findSpectralTerm(std::string imageName, int term, bool doCalc);

                protected:

                    /// @brief A flag indicating whether the source is on the boundary of a subimage.
                    bool atEdge;

                    /// @brief A flag indicating whether a fit has been made to the source.
                    bool hasFit;

                    /// @brief The FITS header information (including WCS and beam info).
                    duchamp::FitsHeader itsHeader;

                    /// @brief The noise level in the vicinity of the object, used for Gaussian fitting
                    float itsNoiseLevel;

                    /// @brief The detection threshold used for the object
                    float itsDetectionThreshold;

                    /// @brief The set of best fit results for different types of fits, plus the overall best
                    std::map<std::string, FitResults> itsBestFitMap;

                    /// @brief The type of the best fit
                    std::string itsBestFitType;

                    /// @brief The parameters used to control the fitting
                    FittingParameters itsFitParams;

                    /// @brief The extent of the box, taking into account the borders of the data array
                    casa::Slicer itsBox;

                    /// @brief The spectral indices of the source components
                    std::map<std::string, std::vector<float> > itsAlphaMap;
                    /// @brief The spectral curvature of the source components
                    std::map<std::string, std::vector<float> > itsBetaMap;

            };


	    /// @brief An analogue of the duchamp::SortDetections function
	    void SortDetections(std::vector<RadioSource> &sourcelist, std::string parameter);


        }

    }

}



#endif
