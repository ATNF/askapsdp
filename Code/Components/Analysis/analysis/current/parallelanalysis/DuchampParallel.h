/// @file
///
/// Provides generic methods for parallel algorithms
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef ASKAP_ANALYSIS_DUCHAMPPARALLEL_H_
#define ASKAP_ANALYSIS_DUCHAMPPARALLEL_H_

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Fitter.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <parallelanalysis/Weighter.h>
#include <preprocessing/VariableThresholder.h>

#include <analysisparallel/SubimageDef.h>

#include <askapparallel/AskapParallel.h>

#include <Common/ParameterSet.h>

#include <duchamp/Cubes/cubes.hh>
#include <duchamp/PixelMap/Voxel.hh>

#include <casa/aipstype.h>
#include <images/Images/SubImage.h>

#include <vector>
#include <string>
#include <map>

namespace askap {
    namespace analysis {

      /// @brief An enumeration describing what data we're after when reading CASA images
      enum DATATYPE { IMAGE, METADATA};

/*       /// Comparison operator for Voxels */
/*       struct voxComp { */
/* 	bool operator() (const PixelInfo::Voxel &lhs, const PixelInfo::Voxel &rhs) const { */
/* 	  if(lhs.getZ()!=rhs.getZ()) return lhs.getZ()<rhs.getZ(); */
/* 	  else if(lhs.getY()!=rhs.getY()) return lhs.getY()<rhs.getY(); */
/* 	  else return lhs.getX()<rhs.getX(); */
/* 	} */
/*       }; */

        /// @brief Support for parallel source finding
        ///
        /// @details This class allows the source finding to be carried out in a
        /// parallel setting.
        /// The model used is that the application has many workers and
        /// one master, running in separate MPI processes or in one single
        /// thread. The master is the master so the number of processes is one
        /// more than the number of workers.
        ///
        /// If the number of nodes is 1 then everything occurs in the same process.
        ///
        /// @ingroup parallelanalysis
        class DuchampParallel {
            public:

                /// @brief Constructor
                /// @details The command line inputs are needed solely for MPI - currently no
                /// application specific information is passed on the command line.
                /// @param argc Number of command line inputs
                /// @param argv Command line inputs
                /// @param parset The parameter set to read Duchamp and other parameters from.
                DuchampParallel(askap::askapparallel::AskapParallel& comms, const LOFAR::ParameterSet& parset);

                /// @brief Default constructor
                DuchampParallel(askap::askapparallel::AskapParallel& comms);

		/// @brief Warning messages about previously-available parameters that are no longer used.
		void deprecatedParameters();
		/// @brief The parset, as used
		LOFAR::ParameterSet &parset(){return itsParset;};

                virtual ~DuchampParallel() {};

                /// @brief Return a reference to the duchamp::Cube object
                duchamp::Cube &cube() {duchamp::Cube &rcube = itsCube; return rcube;};
		duchamp::Cube *pCube(){return &itsCube;};
		
		analysisutilities::SubimageDef &subimageDef(){analysisutilities::SubimageDef &rsubdef = itsSubimageDef; return rsubdef;};

		void setBaseSubsection(std::string sec){itsBaseSubsection = sec;};
		std::string baseSubsection(){return itsBaseSubsection;};
		void setBaseStatSubsection(std::string sec){itsBaseStatSubsection = sec;};
		std::string baseStatSubsection(){return itsBaseStatSubsection;};

                /// @brief Get/set the flag saying whether to do median filtering prior to searching
                /// @{
                bool getFlagVariableThreshold() {return itsFlagVariableThreshold;};
                void setFlagVariableThreshold(bool f) {itsFlagVariableThreshold = f;};
                /// @}

	        /// @brief Make sure the spectral index/curvature images have appropriate names.
	        void checkSpectralTermImages();

                /// @brief Perform a search based on a median box sliding function
                void medianSearch2D();
                void medianSearch();

		void weightSearch();

                /// @brief Read the metadata only from the image file.
                int getMetadata();

                /// @brief Return the beam parameters (in degrees units)
                std::vector<float> getBeamInfo();

                /// @brief Read in the data from the image file (on the workers)
                void readData();
		duchamp::OUTCOME getCASA(DATATYPE typeOfData, bool useSubimageInfo=true);
		const SubImage<Float>* getSubimage(const ImageInterface<Float>* imagePtr, bool useSubimageInfo=true);
		duchamp::OUTCOME getCasaMetadata(const ImageInterface<Float>*  imagePtr, DATATYPE typeOfData);

                /// @brief Set up the log file
                void setupLogfile(int argc, const char** argv);

                /// @brief Condense the lists (on the master)
                void condenseLists();

                /// @brief Sort out the fluxes for all detected objects (on the master)
                void calcFluxes();

                /// @brief Print out the resulting source list (on the master)
                void printResults();

                /// @brief Fit the detected sources (on the workers)
                void fitSources();
		/// @brief Fit a single source
		void fitSource(sourcefitting::RadioSource &src, bool useArray);

		/// @brief Run any preprocessing on the workers
		void preprocess();

                /// @brief Find the sources on each worker
                void findSources();
		void finaliseDetection();

                /// @brief Send the detected sources from the workers to the master
                void sendObjects();

                /// @brief Receive the detected sources from the workers (on the master)
                void receiveObjects();

                /// @brief Fit the sources on the boundaries between workers' subimages (on the master)
                void cleanup();
		void fitRemaining();
		void distributeVoxelList();

		/// @brief Extract spectra/moment-maps/cubelets from a cube for each detected object
		void extract();

                /// @brief Calculate the object parameters on the workers
                void calcObjectParams();

                /// @brief Write a Karma/DS9/CASA annotation files showing the fits (on the master).
                void writeFitAnnotations();

		void writeToFITS();
		
		/// @brief Write an array to an image
		void writeImage(std::string imageName, Float* data);
		void writeImage(std::string imageName, casa::Array<Float> data);

                /// @brief Front end for the statistics functions
                void gatherStats();
                /// @brief Set the desired threshold
                void setThreshold();

                /// @brief Is the dataset a 2-dimensional image?
                bool is2D();

                /// @brief Print out the worker number in form useful for logging.
                std::string workerPrefix(){return printWorkerPrefix(itsComms);};

                /// @brief Get a particular RadioSource
                sourcefitting::RadioSource getSource(int i) {return itsSourceList[i];};
		
		std::vector<sourcefitting::RadioSource> *pSourceList(){return &itsSourceList;};
		std::vector<sourcefitting::RadioSource> sourceList(){return itsSourceList;};

		std::string getSubimageAnnotationFile()      {return itsSubimageAnnotationFile;};
		void        setSubimageAnnotationFile(std::string s){itsSubimageAnnotationFile=s;};
		std::string getFitSummaryFile()      {return itsFitSummaryFile;};
		void        setFitSummaryFile(std::string s){itsFitSummaryFile=s;};
		std::string getFitAnnotationFile()      {return itsFitAnnotationFile;};
		void        setFitAnnotationFile(std::string s){itsFitAnnotationFile=s;};
		std::string getFitBoxAnnotationFile()      {return itsFitBoxAnnotationFile;};
		void        setFitBoxAnnotationFile(std::string s){itsFitBoxAnnotationFile=s;};

            protected:

		/// @brief Check for the existence of deprecated parameters in the parset.
		void checkAndWarn(std::string oldParam, std::string newParam="");


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
		Weighter *itsWeighter;

                /// The Cube of data, which contains the list of Detections.
                duchamp::Cube itsCube;

                /// Shall we search with a varying threshold?
                bool itsFlagVariableThreshold;
		
		// How we do the variable-threshold searching
		VariableThresholder *itsVarThresher;

		/// Whether to extract spectra of detected sources, POSSUM-style
		bool itsFlagExtractSpectra;
		
		/// Whether to extract noise spectra for detected sources, POSSUM-style
		bool itsFlagExtractNoiseSpectra;
		
                /// The list of voxels encompassing detected sources (only for those on the edges of subimages), with fluxes.
                std::vector<PixelInfo::Voxel> itsVoxelList;
		std::map<PixelInfo::Voxel,float> itsVoxelMap;

                /// The list of voxels for edge sources that encodes the S/N ratio (only used when itsFlagVariableThreshold is true)
                std::vector<PixelInfo::Voxel> itsSNRVoxelList;

                /// The Gaussian Fitting parameter class
                sourcefitting::FittingParameters itsFitParams;

		/// Shall the fitting be delegated to the workers?
		bool itsFlagDistribFit;

                /// Shall we find spectral index/curvature information?
		std::vector<bool> itsFlagFindSpectralTerms;
                /// Where shall we find spectral index/curvature information?
		std::vector<string> itsSpectralTermImages;

                /// Name of the summary file
                std::string itsFitSummaryFile;

                /// Name of the Karma annotation file showing the subimages used when in parallel
                std::string itsSubimageAnnotationFile;

                /// Name of the Karma annotation file with the fitted Gaussian components
                std::string itsFitAnnotationFile;
                /// Name of the Karma annotation file with the boxes surrounding the fitted Gaussian components
                std::string itsFitBoxAnnotationFile;

                /// The list of fits to the detected sources.
                std::vector<sourcefitting::RadioSource> itsSourceList;

		/// The list of edge sources
		std::vector<sourcefitting::RadioSource> itsEdgeSourceList;

                /// The definition of the subimage being used (only relevant for the workers)
		analysisutilities::SubimageDef itsSubimageDef;

		/// Use the new mask optimisation growing function?
		bool itsFlagOptimiseMask;

		/// Use the 2D1D wavelet reconstruction algorithm?
		bool itsFlagWavelet2D1D;

        };

    }
}
#endif
