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
#include <analysisutilities/SubimageDef.h>
#include <parallelanalysis/Weighter.h>

#include <mwcommon/AskapParallel.h>

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
                DuchampParallel(askap::mwcommon::AskapParallel& comms, const LOFAR::ParameterSet& parset);

                /// @brief Default constructor
                DuchampParallel(askap::mwcommon::AskapParallel& comms);

                virtual ~DuchampParallel() {};

                /// @brief Return a reference to the duchamp::Cube object
                duchamp::Cube &cube() {duchamp::Cube &rcube = itsCube; return rcube;};

                /// @brief Get/set the flag saying whether to do median filtering prior to searching
                /// @{
                bool getFlagDoMedianSearch() {return itsFlagDoMedianSearch;};
                void setFlagDoMedianSearch(bool f) {itsFlagDoMedianSearch = f;};
                /// @}

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
		void prepareSourceForFit(sourcefitting::RadioSource &src, bool useArray);
		void fitSource(sourcefitting::RadioSource &src, bool useArray);

                /// @brief Find the sources on each worker
                void findSources();

                /// @brief Send the detected sources from the workers to the master
                void sendObjects();

                /// @brief Receive the detected sources from the workers (on the master)
                void receiveObjects();

                /// @brief Fit the sources on the boundaries between workers' subimages (on the master)
                void cleanup();
		void fitRemaining();
		void distributeVoxelList();

                /// @brief Calculate the object parameters on the master.
                void calcObjectParams();
                void calcObjectParamsOLD();

                /// @brief Write a Karma annotation file showing the fits (on the master).
                void writeFitAnnotation();
		
		/// @brief Write an array to an image
		void writeImage(std::string imageName, Float* data);
		void writeImage(std::string imageName, casa::Array<Float> data);

                /// @brief Find the mean (on the workers)
                void findMeans();
                /// @brief Find the STDDEV (on the workers)
                void findStddevs();
                /// @brief Combine and print the mean (on the master)
                void combineMeans();
                /// @brief Send the overall mean to the workers (on the master)
                void broadcastMean();
                /// @brief Combine and print the STDDEV (on the master)
                void combineStddevs();
                /// @brief Front end for the statistics functions
                void gatherStats();

                /// @brief Send the desired threshold to each of the workers (on the master)
                void broadcastThreshold();
                /// @brief Read the threshold to be used (on the workers)
                void receiveThreshold();

                /// @brief Is the dataset a 2-dimensional image?
                bool is2D();

                /// @brief Print out the worker number in form useful for logging.
                std::string workerPrefix(){return printWorkerPrefix(itsComms);};

                /// @brief Set the doFit flag
                void setDoFitFlag(bool f) {itsFlagDoFit = f;};

                /// @brief Get a particular RadioSource
                sourcefitting::RadioSource getSource(int i) {return itsSourceList[i];};

		std::string getSubimageAnnotationFile()      {return itsSubimageAnnotationFile;};
		void        setSubimageAnnotationFile(std::string s){itsSubimageAnnotationFile=s;};
		std::string getFitSummaryFile()      {return itsFitSummaryFile;};
		void        setFitSummaryFile(std::string s){itsFitSummaryFile=s;};
		std::string getFitAnnotationFile()      {return itsFitAnnotationFile;};
		void        setFitAnnotationFile(std::string s){itsFitAnnotationFile=s;};
		std::string getFitBoxAnnotationFile()      {return itsFitBoxAnnotationFile;};
		void        setFitBoxAnnotationFile(std::string s){itsFitBoxAnnotationFile=s;};

            protected:

                // Class for communications
                askap::mwcommon::AskapParallel& itsComms;

                /// Is the image a FITS file or not (if not, probably a casa image...)
                bool itsIsFITSFile;

		/// The base subsection of the image;
		std::string itsBaseSubsection;

		/// The base statistics subsection of the image;
		std::string itsBaseStatSubsection;

                /// An image showing relative weights of pixels
                std::string itsWeightImage;

		/// Whether to work out a SNR threshold for each individual subimage.
		bool itsFlagThresholdPerWorker;

		/// The weighting of each pixel
		Weighter *itsWeighter;

                /// The weights values
                casa::Vector<casa::Double> itsWeights;

                /// The Cube of data, which contains the list of Detections.
                duchamp::Cube itsCube;

                /// Shall we search after median-smoothing?
                bool itsFlagDoMedianSearch;

                /// The half-width of the box used for median filtering
                int itsMedianBoxWidth;

		/// Whether to write a casa image containing the S/N ratio
		bool itsFlagWriteSNRimage;

		/// Name of S/N image to be written
		std::string itsSNRimageName;
		
                /// The list of voxels encompassing detected sources (only for those on the edges of subimages), with fluxes.
                std::vector<PixelInfo::Voxel> itsVoxelList;
/*                 std::map<PixelInfo::Voxel,float,voxComp> itsVoxelMap; */
		std::map<PixelInfo::Voxel,float> itsVoxelMap;

                /// The list of voxels for edge sources that encodes the S/N ratio (only used when itsFlagDoMedianSearch is true)
                std::vector<PixelInfo::Voxel> itsSNRVoxelList;

                /// The Gaussian Fitting parameter class
                sourcefitting::FittingParameters itsFitParams;

                /// Shall we fit to the sources?
                bool itsFlagDoFit;

                /// Shall we fit to just the detected voxels?
                bool itsFlagFitJustDetection;

		bool itsFlagDistribFit;

                /// Shall we find spectral index information?
                bool itsFlagFindSpectralIndex;

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

                /// The definition of the subimage being used (only relevant for the workers)
                SubimageDef itsSubimageDef;

        };

    }
}
#endif
