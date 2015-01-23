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
#include <analysisparallel/SubimageDef.h>

#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/param.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Outputs/columns.hh>
#include <duchamp/Outputs/AnnotationWriter.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/Outputs/CatalogueSpecification.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian2D.h>

#include <casa/namespace.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <boost/shared_ptr.hpp>

#include <map>
#include <vector>
#include <utility>
#include <string>

//using namespace duchamp;

namespace askap {

namespace analysis {

class DuchampParallel;

namespace sourcefitting {


/// @brief Class to store all information on a detected source.
///
/// @details This class is designed to hold all appropriate
/// information on a source detected in an image or cube. It derives
/// from the duchamp::Detection class, and so records the pixel and
/// world coordinate information, as well as the pixel-based flux
/// information (peak flux, total flux, etc). However the RadioSource
/// class is designed to be able to fit an object with known functions
/// (primarily gaussians) and store the fitted parameters.
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
        RadioSource& operator= (const duchamp::Detection& det);

        /// @brief Destructor
        virtual ~RadioSource() {};

        /// @details Reimplementing the addOffsets function
        /// from Detection. It runs the Detection version,
        /// then adds the offsets to each of the component
        /// positions.
        void addOffsets(long xoff, long yoff, long zoff);

        void addOffsets()
        {
            addOffsets(this->xSubOffset, this->ySubOffset, this->zSubOffset);
        };
        void removeOffsets(long xoff, long yoff, long zoff)
        {
            addOffsets(-xoff, -yoff, -zoff);
        };
        void removeOffsets()
        {
            addOffsets(-xSubOffset, -ySubOffset, -zSubOffset);
        };
        void addOffsets(duchamp::Param &par)
        {
            setOffsets(par); addOffsets();
        };

        void haveNoParams() {haveParams = false;};

        /// @brief Find the local maxima in the flux distribution of
        /// the Detection.
        /// Find a list of local maxima in the detection. This divides
        /// the flux interval between the object's peak flux and the
        /// detection threshold into 10, and searches for objects at
        /// each of these sub-thresholds. Maxima other than the overall
        /// peak will appear at some thresholds but not others.
        ///
        /// The list of peak locations is returned as a STL multimap,
        /// with the Key element being the number of times a peak
        /// location was found (the overall peak will be found 10
        /// times), and the Value element being the location of the
        /// peak, stored as a PixelInfo::Voxel.
        std::multimap<int, PixelInfo::Voxel>
        findDistinctPeaks(casa::Vector<casa::Double> f);

        /// @brief Estimate the FWHM of the Detection.
        /// @details This returns an estimate of an object's shape,
        /// using the principal axes and position angle calculated in
        /// the duchamp::PixelInfo code. This is done by using the
        /// array of flux values given by f, thresholding at half the
        /// object's peak flux value, and averaging the x- and
        /// y-widths that the Duchamp code gives.
        ///
        /// It may be that the thresholding returns more than one
        /// object. In this case, we only look at the one with the
        /// same peak location as the base object.
        void getFWHMestimate(std::vector<float> fluxarray,
                             double &angle,
                             double &maj,
                             double &min);

        /// @brief Return a list of subcomponents.
        std::vector<SubComponent>
        getSubComponentList(casa::Matrix<casa::Double> pos,
                            casa::Vector<casa::Double> &f);

        /// @brief Return a list of subcomponents that lie above a
        /// flux threshold
        /// @details This function returns a vector list of
        /// subcomponents that make up the Detection. The pixel array
        /// f is searched at a series of thresholds spaced
        /// logarithmically between the Detection's peak flux and the
        /// original detection threshold. If more than one object is
        /// detected at any of these searches, getSubComponentList()
        /// is called on each of these objects.
        ///
        /// This recursive exectution will continue until only
        /// one object is left, at which point we return a
        /// SubComponent object that holds all parameters
        /// necessary to specify a 2D Gaussian (the shape
        /// parameters are determined using
        /// getFWHMestimate()).
        ///
        /// @return The ultimate returned object is a vector
        /// list of SubComponents, ordered from highest to
        /// lowest peak flux.
        std::vector<SubComponent>
        getThresholdedSubComponentList(std::vector<float> fluxarray);

        /// @brief Set necessary parameters for fit
        void prepareForFit(duchamp::Cube &cube, bool useArray);

        /// @brief Fit Gaussian components to the Detection.
        /// @name
        ///@{

        /// @details The principle interface to the Gaussian
        /// fitting. Depending on the choice in the FittingParameters,
        /// this either extracts the flux and dimension arrays from
        /// the cube and passes them to fitGauss(std::vector<float>,
        /// std::vector<size_t>), or extracts the set of voxels that
        /// make up the object and pass them to
        /// fitGauss(std::vector<PixelInfo::Voxel> &). The
        /// FittingParameters need to have been set prior to calling
        /// (via setFitParams).
        bool fitGauss(duchamp::Cube &cube);

        /// @details First defines the pixel array with the
        /// flux values of just the detected pixels by
        /// extracting the voxels from the given
        /// voxelList. Their flux values are placed in the
        /// flux matrix, which is passed to
        /// fitGauss(casa::Matrix<casa::Double> pos,
        /// casa::Vector<casa::Double> f,
        /// casa::Vector<casa::Double> sigma). The FittingParameters need to have
        /// been set prior to calling (via setFitParams).
        bool fitGauss(std::vector<PixelInfo::Voxel> &voxelList);

        /// @details First defines the pixel array with the flux
        /// values by extracting the voxels from fluxArray that are
        /// within the box surrounding the object. Their flux values
        /// are placed in the flux matrix, which is passed to
        /// fitGauss(casa::Matrix<casa::Double> pos,
        /// casa::Vector<casa::Double> f, casa::Vector<casa::Double>
        /// sigma). The FittingParameters need to have
        /// been set prior to calling (via setFitParams).
        bool fitGauss(std::vector<float> fluxArray,
                      std::vector<size_t> dimArray);

        /// @details This function drives the fitting of the Gaussian
        /// functions. It first sets up the fitting parameters, then
        /// finds the sub-components present in the box. The main loop
        /// is over the requested fit types. For each valid type, the
        /// Fitter is initialised and run for a number of gaussians
        /// from 1 up to the maximum number requested.
        ///
        /// Each fit is compared to the best thus far, whose
        /// properties are tracked for later use. The best fit for
        /// each type is saved for later writing to the appropriate
        /// summary file, and the best overall is saved in itsBestFit
        ///
        /// The FittingParameters need to have
        /// been set prior to calling (via setFitParams). This
        /// function changes them by setting the specific values of
        /// the peak flux, detection threshold, and box.
        ///
        /// @return The return value is the value of itsFlagHasFit,
        /// which indicates whether a valid fit was made.
        bool fitGauss(casa::Matrix<casa::Double> pos,
                      casa::Vector<casa::Double> f,
                      casa::Vector<casa::Double> sigma);
        ///@}

        /// @brief Store the FITS header information
        void setHeader(const duchamp::FitsHeader &head) {itsHeader = head;};

        /// @brief Get the FITS header information
        duchamp::FitsHeader &header() {return itsHeader;};

        /// @brief Functions to set the object's noise level
        /// @name
        /// @{

        /// @brief Set the noise level to the local value, using an
        /// array.  @details Sets the value of the local noise level
        /// by taking the MADFM of the surrounding pixels from the
        /// Cube's array.  A box of side length boxSize is centred on
        /// the peak pixel of the detection, and the MADFM of the
        /// pixels therein is found. This is converted to a Gaussian
        /// rms, and stored as the RadioSource::itsNoiseLevel value.
        /// @param array Array of pixel values @param dim Set of
        /// dimensions for array @param boxSize The side length of the
        /// box used.
        void setNoiseLevel(std::vector<float> &array,
                           std::vector<size_t> &dim,
                           unsigned int boxSize = defaultNoiseBoxSize);

        /// @brief Set the noise level to the local value, using a
        /// Duchamp::Cube object.
        /// @details Sets the value of the local noise level by taking
        /// the MADFM of the surrounding pixels from the Cube's array.
        /// Calls setNoiseLevel(vector<float>, vector<size_t>, int).
        ///@param cube The duchamp::Cube object containing the pixel
        /// array
        void setNoiseLevel(duchamp::Cube &cube);

        /// @brief Set the noise level
        void setNoiseLevel(float noise) {itsNoiseLevel = noise;};
        float noiseLevel() {return itsNoiseLevel;};
        /// @}

        /// @brief Set the detection threshold for a particular Cube
        void setDetectionThreshold(duchamp::Cube &cube,
                                   bool flagMedianSearch,
                                   std::string snrImage = "");

        /// @brief Set the detection threshold directly
        void setDetectionThreshold(float threshold) {itsDetectionThreshold = threshold;};

        void setDetectionThreshold(std::vector<PixelInfo::Voxel> &inVoxlist,
                                   std::vector<PixelInfo::Voxel> &inSNRvoxlist,
                                   bool flagMedianSearch);

        /// @brief Return the detection threshold
        float detectionThreshold() {return itsDetectionThreshold;};

        /// @brief Return the set of fits
        FitResults fitResults(std::string type)
        {
            return itsBestFitMap[type];
        };
        std::vector<casa::Gaussian2D<Double> > gaussFitSet(std::string type)
        {
            return itsBestFitMap[type].fitSet();
        };
        std::vector<casa::Gaussian2D<Double> > gaussFitSet()
        {
            return itsBestFitMap["best"].fitSet();
        };

        /// @brief Return the number of fits for a fit type
        unsigned int numFits(std::string type)
        {
            return itsBestFitMap[type].numFits();
        };

        /// @brief Return the number of fits for the best set
        unsigned int numFits()
        {
            return itsBestFitMap["best"].numFits();
        };

        std::vector<float> alphaValues(std::string type) {return itsAlphaMap[type];};
        std::vector<float> betaValues(std::string type) {return itsBetaMap[type];};

        /// @brief Return a reference to the set of Gaussian fits.
        std::vector<casa::Gaussian2D<Double> >& fitset(std::string type)
        {
            return itsBestFitMap[type].fits();
        };

        ///  Print a row of values for the current Detection into an
        ///  output table. Columns are printed according to the
        ///  tableType string, using the Column::doCol() function as a
        ///  determinant.  \param stream Where the output is written
        ///  \param columns The vector list of Column objects \param
        ///  tableType A Catalogues::DESTINATION label saying what
        ///  format to use: one of FILE, LOG, SCREEN or VOTABLE
        ///  (although the latter shouldn't be used with this
        ///  function).
        void printTableRow(std::ostream &stream,
                           duchamp::Catalogues::CatalogueSpecification columns,
                           size_t fitNum = 0,
                           std::string fitType = "best");

        ///  Print a single value into an output table. The
        ///  Detection's correct value is extracted according to the
        ///  Catalogues::COLNAME key in the column given.  \param
        ///  stream Where the output is written \param column The
        ///  Column object defining the formatting.
        void printTableEntry(std::ostream &stream,
                             duchamp::Catalogues::Column column,
                             size_t fitNum = 0,
                             std::string fitType = "best");

        /// @brief Write the description of the fits to an annotation
        /// file.
        /// This function writes the information about the fitted
        /// Gaussian components to a given annotation file. There are
        /// two different elements drawn for each RadioSource object -
        /// the half-maximum ellipse, and, optionally, the box given
        /// by FittingParameters::boxPadSize(). We use the Duchamp
        /// annotation file interface, to allow karma, DS9 and CASA
        /// formats to be written.
        void writeFitToAnnotationFile(boost::shared_ptr<duchamp::AnnotationWriter> &writer,
                                      int sourceNum,
                                      bool doEllipse,
                                      bool doBox);

        /// @brief Functions allowing RadioSource objects to be passed
        /// over LOFAR Blobs
        /// @name
        /// @{

        /// @brief Pass a RadioSource object into a Blob
        /// @brief This function provides a mechanism for passing the
        /// entire contents of a RadioSource object into a
        /// LOFAR::BlobOStream stream
        friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &stream,
                                              RadioSource& src);
        /// @brief Receive a RadioSource object from a Blob
        /// @brief Receive a RadioSource object from a Blob
        /// @details This function provides a mechanism for receiving the
        /// entire contents of a RadioSource object from a
        /// LOFAR::BlobIStream stream
        friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &stream,
                                              RadioSource& src);

        /// @}

        /// @brief Is the object at the edge of a subimage
        bool isAtEdge() {return itsFlagAtEdge;};
        /// @brief Set the itsFlagAtEdge flag.
        void setAtEdge(bool b) {itsFlagAtEdge = b;};

        /// @brief Set the itsFlagAtEdge flag using information from a
        /// Cube @details Sets the itsFlagAtEdge flag based on the
        /// dimensions of the cube and the duchamp parameters
        /// flagAdjacent, threshS and threshV. If flagAdjacent is
        /// true, then the source is at the edge if it occupies a
        /// pixel on the boundary of the image (the z-direction is
        /// only examined if there is more than one channel). If
        /// flagAdjacent, the source must lie within the appropriate
        /// threshold (threshS for the spatial directions and threshV
        /// for the spectral/velocity) of the image boundary.
        ///
        /// The image boundary here takes into account the size of any
        /// overlap region between neighbouring subimages, but only
        /// for image sides that have a neighbour (for those on the
        /// edge of the full image, the boundary is assumed to be the
        /// image boundary).
        ///
        /// @param cube The duchamp::Cube object that holds the
        /// dimensions and parameters
        /// @param subimage The SubimageDef object that holds the
        /// information on the number of subimages & their overlap.
        /// @param workerNum The number of the worker in question,
        /// starting at 0 (which subimage are we in?)
        void setAtEdge(duchamp::Cube &cube,
                       analysisutilities::SubimageDef &subimage,
                       int workerNum);

        /// @brief Define the boundaries of the box
        /// @details Defines the maximum and minimum points of the box
        /// in each axis direction. The size of the image array is
        /// taken into account, using the axes array, so that the box
        /// does not go outside the allowed pixel area.
        void defineBox(duchamp::Section &sec, int spectralAxis = 2);

        /// @brief Return a subsection string detailing extent of
        /// object
        /// @details This function returns a subsection string
        /// that shows the bounding box for the object. This
        /// will be in a suitable format for use with the
        /// subsection string in the input parameter set. It
        /// uses the FitsHeader object to know which axis
        /// belongs where.
        std::string boundingSubsection(std::vector<size_t> dim,
                                       bool fullSpectralRange);

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
        size_t boxXsize() {return itsBox.length()[0];};
        /// Y-width
        size_t boxYsize() {return itsBox.length()[1];};
        /// Number of pixels in box
        size_t boxSize() {return boxXsize() * boxYsize();};

        /// Return the full box description
        casa::Slicer box() {return itsBox;};
        /// Define the box in one shot
        void setBox(casa::Slicer box) {itsBox = box;};

        // @}

        /// @brief Comparison operator, using the name field
        friend bool operator< (RadioSource lhs, RadioSource rhs)
        {
            if (lhs.getZcentre() == rhs.getZcentre()) {
                return (lhs.name < rhs.name);
            } else {
                return (lhs.getZcentre() < rhs.getZcentre());
            }
        }

        /// @brief Return a reference to the fitting parameters
        const FittingParameters &fitparams()
        {
            const FittingParameters& rfitpars = itsFitParams;
            return rfitpars;
        };

        /// @brief Set the fitting parameters by passing a set
        void setFitParams(const FittingParameters &fitpars)
        {
            itsFitParams = fitpars;
        };

        /// @brief Find the spectral index or curvature for each
        /// fitted component in the source
        /// @details This function finds the value of the spectral
        /// index or spectral curvature for each Gaussian component
        /// fitted to the zeroth Taylor term image. The procedure is:
        /// @li Find the Taylor 1/2 image from the provided image name
        /// (must be of format *.taylor.0*)
        /// @li Extract pixel values within the source's box
        /// @li For each Gaussian component of the source, and for
        /// each fit type, fit the same shape & location Gaussian
        /// (that is, only fit the height of the Gaussian).
        /// @li Calculate the spectral index or curvature using the
        /// appropriate formulae
        /// @li Store the spectral index value in a map indexed by fit type.
        /// Note that if the imageName provided is not of the correct
        /// format, nothing is done.
        /// @param imageName The name of the image from which to
        /// extract the spectral information
        /// @param term Which Taylor term to do - either 1 or 2, other
        /// values trigger an exception
        /// @param doCalc If true, do all the calculations. If false,
        /// fill the alpha & beta values to 0. for all fit types.
        void findSpectralTerm(std::string imageName, int term, bool doCalc);

    protected:

        /// @brief A flag indicating whether the source is on the
        /// boundary of a subimage.
        bool itsFlagAtEdge;

        /// @brief A flag indicating whether a fit has been made to
        /// the source.
        bool itsFlagHasFit;

        /// @brief The FITS header information (including WCS and beam
        /// info).
        duchamp::FitsHeader itsHeader;

        /// @brief The noise level in the vicinity of the object, used
        /// for Gaussian fitting
        float itsNoiseLevel;

        /// @brief The detection threshold used for the object
        float itsDetectionThreshold;

        /// @brief The set of best fit results for different types of
        /// fits, plus the overall best
        std::map<std::string, FitResults> itsBestFitMap;

        /// @brief The type of the best fit
        std::string itsBestFitType;

        /// @brief The parameters used to control the fitting.
        /// @details These are defined externally and set, then altered to
        /// include the specific box used in the fitting, as well as the
        /// source's peak flux & detection threshold.
        FittingParameters itsFitParams;

        /// @brief The extent of the box, taking into account the
        /// borders of the data array
        casa::Slicer itsBox;

        /// @brief The spectral indices of the source components
        std::map<std::string, std::vector<float> > itsAlphaMap;

        /// @brief The spectral curvature of the source components
        std::map<std::string, std::vector<float> > itsBetaMap;

};

}

}

}



#endif
