/// @file
///
/// Provides a distributed interface to source finding with the duchamp Cube structure.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_DUCHAMPPARALLEL_H_
#define ASKAP_ANALYSIS_DUCHAMPPARALLEL_H_

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Fitter.h>
#include <parallelanalysis/Weighter.h>
#include <preprocessing/VariableThresholder.h>

#include <analysisparallel/SubimageDef.h>

#include <askapparallel/AskapParallel.h>

#include <Common/ParameterSet.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <duchamp/Cubes/cubes.hh>
#include <duchamp/PixelMap/Voxel.hh>

#include <casa/aipstype.h>
#include <images/Images/SubImage.h>

#include <vector>
#include <string>
#include <map>

namespace askap {
namespace analysis {

/// @brief An enumeration describing what data we're after when
/// reading CASA images
enum DATATYPE { IMAGE, METADATA};

/// @brief Support for parallel source finding
///
/// @details This class allows the source finding to be carried out in
/// a parallel setting.  The model used is that the application has
/// many workers and one master, running in separate MPI processes or
/// in one single thread. The master is the master so the number of
/// processes is one more than the number of workers.
///
/// If the number of nodes is 1 then everything occurs in the same
/// process.
///
/// @ingroup parallelanalysis
class DuchampParallel {
    public:

        /// @brief Constructor
        /// @details The constructor reads parameters from the parameter
        /// set parset. This set can include Duchamp parameters, as well
        /// as particular Selavy parameters such as masterImage and
        /// sectionInfo.
        DuchampParallel(askap::askapparallel::AskapParallel& comms,
                        const LOFAR::ParameterSet& parset);

        /// @brief Default constructor
        DuchampParallel(askap::askapparallel::AskapParallel& comms);

        /// @brief Warning messages about previously-available
        /// parameters that are no longer used.
        /// @details A check is made for the presence in the
        /// parset of parameters that have been deprecated. The
        /// parset is updated if need be according to the rules
        /// for DuchampParallel::checkAndWarn().
        void deprecatedParameters();
        /// @brief The parset, as used
        LOFAR::ParameterSet &parset() {return itsParset;};

        virtual ~DuchampParallel() {};

        /// @brief Return a reference to the duchamp::Cube object
        duchamp::Cube &cube() {duchamp::Cube &rcube = itsCube; return rcube;};
        duchamp::Cube *pCube() {return &itsCube;};

        analysisutilities::SubimageDef &subimageDef()
        {
            analysisutilities::SubimageDef &rsubdef = itsSubimageDef;
            return rsubdef;
        };

        void setBaseSubsection(std::string sec) {itsBaseSubsection = sec;};
        std::string baseSubsection() {return itsBaseSubsection;};
        void setBaseStatSubsection(std::string sec) {itsBaseStatSubsection = sec;};
        std::string baseStatSubsection() {return itsBaseStatSubsection;};

        /// @brief Get/set the flag saying whether to do median
        /// filtering prior to searching
        ///@{
        bool getFlagVariableThreshold() {return itsFlagVariableThreshold;};
        void setFlagVariableThreshold(bool f) {itsFlagVariableThreshold = f;};
        /// @}
        // VariableThresholder *varThresher() {return itsVarThresher.get();};
        boost::shared_ptr<VariableThresholder> varThresher() {return itsVarThresher;};

        sourcefitting::FittingParameters &fitParams()
        {
            sourcefitting::FittingParameters &rFitPars = itsFitParams;
            return rFitPars;
        };


        /// @brief Make sure the spectral index/curvature images have
        /// appropriate names.
        /// @details Once the parameters relating to the spectral
        /// index & curvature images have been read, we need to
        /// check to see if the images need to be specified.
        void checkSpectralTermImages();

        /// @brief Set up the Subimage definition for the case of a
        /// FITS file.
        /// @details This utility function sets up the SubimageDef
        /// object appropriate for the case that we are accessing
        /// a FITS file. Upon completion, the SubimageDef object
        /// will have its image name, subsection string, image
        /// dimensions and nsub/overlap parameters defined. If no
        /// subsectioning is required, the subsection string in
        /// the cube parameters will be set to the null subsection
        /// of appropriate dimensionality.
        void setSubimageDefForFITS();

        /// @brief Read the metadata only from the image file.
        /// @details Provides a simple front-end to the correct
        /// metadata-reading function, according to whether the image is
        /// FITS data or a CASA image
        /// @return The return value of the function that was used:
        /// either duchamp::SUCCESS or duchamp::FAILURE
        int getMetadata();

        /// @brief Return the beam parameters (in degrees units)
        /// @details Returns a vector containing the beam parameters:
        /// major axis [deg], minor axis [deg], position angle [deg].
        std::vector<float> getBeamInfo();

        /// @brief Read in the data from the image file (on the workers)
        /// @details Reads in the data to the duchamp::Cube class. For
        /// the workers, this either uses the duchamp functionality, in
        /// the case of FITS data, or calls the routines in
        /// CasaInterface.cc in the case of casa (or other) formats. If
        /// reconstruction or smoothing are required, they are done in
        /// this function. For the master, the metadata only is read
        /// from the file, with the same choice based on the FITS status
        /// of the data file.
        void readData();

        /// @details This is the front-end to the image-access
        /// functionality for CASA images. It replicates (kinda) the
        /// behaviour of duchamp::Cube::getCube(). First the image is
        /// opened, then we get the metadata for the image via
        /// getCasaMetadata. Then the subimage that we want is defined
        /// (including the parsing of any subsections given in the
        /// parset), then, if we request IMAGE data, the actual pixel
        /// values are read from the image and stored in the itsCube
        /// object.
        /// @param typeOfData What sort of data are we after? Image
        /// data or just Metadata?
        /// @param useSubimageInfo Whether to use the information of
        /// the distributed nature of the data to determine the
        /// subimage shape, or whether just to get the whole image
        /// dimensions.
        /// @return duchamp::SUCCESS if successful, duchamp::FAILURE
        /// otherwise.
        duchamp::OUTCOME getCASA(DATATYPE typeOfData,
                                 bool useSubimageInfo = true);

        /// @details Define the shape/size of the subimage being used,
        /// and return a pointer that can be used to extract the image
        /// data. The subimage is defined by the itsSubimageDef
        /// object, which, when useSubimageInfo=true, takes into
        /// account how the image is distributed amongst workers. If
        /// useSubimageInfo=false, the whole image is considered. The
        /// image subsection and the statistics subsection are both
        /// parsed and tested for validity.
        /// @param imagePtr Pointer to the image - needs to be opened
        /// and valid
        /// @param useSubimageInfo Whether to use infomation about the
        /// distribution of the image amongst workers.
        /// @return A casa::SubImage pointer to the desired sub-image.
        const boost::shared_ptr<SubImage<Float> >
        getSubimage(const boost::shared_ptr<ImageInterface<Float> > imagePtr,
                    bool useSubimageInfo = true);

        /// @details Read some basic metadata from the image, storing
        /// the WCS information, beam information, flux units, setting
        /// the is2D flag and fixing spectral units if need be. If we
        /// want METADATA, then the cube is initialised without
        /// allocation (ie. the dimension array is set and some
        /// parameter flags are checked). Otherwise, initialisation is
        /// saved till later.
        /// @param imagePtr The image, already opened
        /// @param typeOfData Either IMAGE or METADATA
        duchamp::OUTCOME
        getCasaMetadata(const boost::shared_ptr<ImageInterface<Float> >  imagePtr,
                        DATATYPE typeOfData);

        /// @brief Set up the log file
        /// @details Opens the log file and writes the execution
        /// statement, the time, and the duchamp parameter set to it.
        void setupLogfile(int argc, const char** argv);

        /// @brief Print out the resulting source list (on the master)
        /// @details The final list of detected objects is written to
        /// the terminal and to the results file in the standard Duchamp
        /// manner.
        void printResults();

        /// @brief Fit the detected sources (on the workers)
        /// @details The list of RadioSource objects is populated: one
        /// for each of the detected objects. If the 2D profile fitting
        /// is requested, all sources that are not on the image boundary
        /// are fitted by the RadioSource::fitGauss(float *, long *)
        /// function. The fitting for those on the boundary is left for
        /// the master to do after they have been combined with objects
        /// from other subimages.
        ///
        /// @todo Make the boundary determination smart enough to know
        /// which side is adjacent to another subimage.
        void fitSources();

        /// @brief Fit a single source
        void fitSource(sourcefitting::RadioSource &src);

        /// @brief Run any preprocessing on the workers
        /// @details Runs any requested pre-processing. This includes
        /// inverting the cube, smoothing or multi-resolution wavelet
        /// reconstruction.  This is only done on the worker nodes.
        void preprocess();

        /// @brief Find the sources on each worker
        /// @details Searches the image/cube for objects, using the
        /// appropriate search function given the user
        /// parameters. Merging of neighbouring objects is then done,
        /// and all WCS parameters are calculated.
        ///
        /// This is only done on the workers, although if we use the
        /// weight or variable-threshold search the master needs to do
        /// the initialisation of itsWeighter/itsVarThresher
        void findSources();

        /// Remove non-edge sources that are smaller than originally
        /// requested, as these won't be grown any further.
        void finaliseDetection();

        /// @brief Send the detected sources from the workers to the
        /// master
        /// @details The RadioSource objects on each worker, which
        /// contain each detected object, are sent to the Master node
        /// via LOFAR Blobs.
        void sendObjects();

        /// @brief Receive the detected sources from the workers (on
        /// the master)
        /// @details On the Master node, receive the list of RadioSource
        /// objects sent by the workers.
        void receiveObjects();

        /// @brief Fit the sources on the boundaries between workers'
        /// subimages (on the master)
        /// @details Done on the Master node. This function gathers the
        /// sources that are marked as on the boundary of subimages, and
        /// combines them via the duchamp::Cubes::ObjectMerger()
        /// function. The resulting sources are then fitted (if so
        /// required) and have their WCS parameters calculated by the
        /// ObjectParameteriser class
        ///
        /// Once this is done, these sources are added to the cube
        /// detection list, along with the non-boundary objects. The
        /// final list of RadioSource objects is then sorted (by the
        /// Name field) and given object IDs.
        void cleanup();

        /// @brief Extract spectra/moment-maps/cubelets from a cube
        /// for each detected object
        void extract();

        void writeToFITS();

        /// @brief Front end for the statistics functions
        /// @details A front-end function that calls all the
        /// statistics functions. Net effect is to find the
        /// mean/median and stddev/MADFM for the entire dataset and
        /// store these values in the master's itsCube statsContainer.
        void gatherStats();

        /// @brief Set the desired threshold
        /// @details The detection threshold value (which has been
        /// already calculated) is properly set for use. In the
        /// distributed case, this means the master sends it to the
        /// workers via LOFAR Blobs, and the workers individually set
        /// it.
        void setThreshold();

        /// @brief Is the dataset a 2-dimensional image?
        /// @details Check whether the image is 2-dimensional, by
        /// looking at the dim array in the Cube object, and counting
        /// the number of dimensions that are greater than 1 @todo
        /// Make use of the new Cube::is2D() function.
        bool is2D();

        /// @brief Get a particular RadioSource
        sourcefitting::RadioSource getSource(int i) {return itsSourceList[i];};

        std::vector<sourcefitting::RadioSource> *pSourceList()
        {
            return &itsSourceList;
        };
        std::vector<sourcefitting::RadioSource> sourceList()
        {
            return itsSourceList;
        };
        std::vector<sourcefitting::RadioSource> &rSourceList()
        {
            std::vector<sourcefitting::RadioSource> &ref = itsSourceList;
            return ref;
        };
        std::vector<sourcefitting::RadioSource> *pEdgeList()
        {
            return &itsEdgeSourceList;
        };
        std::vector<sourcefitting::RadioSource> edgeList()
        {
            return itsEdgeSourceList;
        };

    protected:

        /// @brief Check for the existence of deprecated parameters in the parset.
        /// @details A utility function to check for the existence
        /// in the parset of an out-of-date parameter (ie. one
        /// that has been deprecated). If it is present, a warning
        /// message is displayed, and, if it has been renamed this
        /// is also conveyed to the user. If the renamed parameter
        /// is not present in the parset, then it is assigned the
        /// value taken by the old one.
        /// @param oldParam The old, deprecated parameter name
        /// @param newParam The new parameter name. If there is no
        /// equivalent, give this as "".
        void checkAndWarn(std::string oldParam, std::string newParam = "");

        // Class for communications
        askap::askapparallel::AskapParallel& itsComms;

        /// The parset
        LOFAR::ParameterSet itsParset;

        /// Is the image a FITS file or not (if not, probably a casa image...)
        bool itsIsFITSFile;

        /// The base subsection of the image;
        std::string itsBaseSubsection;

        /// The base statistics subsection of the image;
        std::string itsBaseStatSubsection;

        /// Whether to work out a SNR threshold for each individual subimage.
        bool itsFlagThresholdPerWorker;

        /// An image showing relative weights of pixels
        bool itsFlagWeightImage;

        /// The weighting of each pixel
        boost::shared_ptr<Weighter> itsWeighter;

        /// The Cube of data, which contains the list of Detections.
        duchamp::Cube itsCube;

        /// Shall we search with a varying threshold?
        bool itsFlagVariableThreshold;

        // How we do the variable-threshold searching
        boost::shared_ptr<VariableThresholder> itsVarThresher;

        /// Whether to extract spectra of detected sources,
        /// POSSUM-style
        bool itsFlagExtractSpectra;

        /// Whether to extract noise spectra for detected sources,
        /// POSSUM-style
        bool itsFlagExtractNoiseSpectra;

        /// The Gaussian Fitting parameter class
        sourcefitting::FittingParameters itsFitParams;

        /// Shall the fitting be delegated to the workers?
        bool itsFlagDistribFit;

        /// Shall we find spectral index/curvature information?
        std::vector<bool> itsFlagFindSpectralTerms;
        /// Where shall we find spectral index/curvature information?
        std::vector<string> itsSpectralTermImages;

        /// The list of fits to the detected sources.
        std::vector<sourcefitting::RadioSource> itsSourceList;

        /// The list of edge sources
        std::vector<sourcefitting::RadioSource> itsEdgeSourceList;

        /// The definition of the subimage being used (only relevant
        /// for the workers)
        analysisutilities::SubimageDef itsSubimageDef;

        /// Use the new mask optimisation growing function?
        bool itsFlagOptimiseMask;

        /// Use the 2D1D wavelet reconstruction algorithm?
        bool itsFlagWavelet2D1D;

};

}
}
#endif
