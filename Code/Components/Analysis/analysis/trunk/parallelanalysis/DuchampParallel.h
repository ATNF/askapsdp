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

#include <askap_analysis.h>

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Fitter.h>
#include <analysisutilities/SubimageDef.h>

#include <askapparallel/AskapParallel.h>

#include <APS/ParameterSet.h>

#include <duchamp/Cubes/cubes.hh>
#include <duchamp/PixelMap/Voxel.hh>

#include <vector>
#include <string>

namespace askap {
    namespace analysis {

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
        class DuchampParallel : public askap::cp::AskapParallel {
            public:

                /// @brief Constructor
                /// @details The command line inputs are needed solely for MPI - currently no
                /// application specific information is passed on the command line.
                /// @param argc Number of command line inputs
                /// @param argv Command line inputs
                /// @param parset The parameter set to read Duchamp and other parameters from.
                DuchampParallel(int argc, const char** argv, const LOFAR::ACC::APS::ParameterSet& parset);

                /// @brief Default constructor
                DuchampParallel(int argc, const char** argv);

                virtual ~DuchampParallel() {};

                duchamp::Cube &cube() {duchamp::Cube &rcube = itsCube; return rcube;};

                /// @brief Read the metadata only from the image file.
                int getMetadata();

                /// @brief Return the beam parameters (in degrees units)
                std::vector<float> getBeamInfo();

                /// @brief Read in the data from the image file (on the workers)
                void readData();

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

                /// @brief Find the sources on each worker
                void findSources();

                /// @brief Send the detected sources from the workers to the master
                void sendObjects();

                /// @brief Receive the detected sources from the workers (on the master)
                void receiveObjects();

                /// @brief Fit the sources on the boundaries between workers' subimages (on the master)
                void cleanup();

                /// @brief Calculate the object parameters on the master.
                void calcObjectParams();

                /// @brief Write a Karma annotation file showing the fits (on the master).
                void writeFitAnnotation();

                /// @brief Find the mean (on the workers)
                void findMeans();
                /// @brief Find the RMS (on the workers)
                void findRMSs();
                /// @brief Combine and print the mean (on the master)
                void combineMeans();
                /// @brief Send the overall mean to the workers (on the master)
                void broadcastMean();
                /// @brief Combine and print the RMS (on the master)
                void combineRMSs();
                /// @brief Front end for the statistics functions
                void gatherStats();

                /// @brief Send the desired threshold to each of the workers (on the master)
                void broadcastThreshold();
                /// @brief Read the threshold to be used (on the workers)
                void receiveThreshold();

                /// @brief Is the dataset a 2-dimensional image?
                bool is2D();

                /// @brief Print out the worker number in form useful for logging.
                std::string workerPrefix();

                /// @brief Set the doFit flag
                void setDoFitFlag(bool f) {itsFlagDoFit = f;};

                /// @brief Get a particular RadioSource
                sourcefitting::RadioSource getSource(int i) {return itsSourceList[i];};

            protected:

                /// The name of the file containing the image data.
                std::string itsImage;

                /// Is the image a FITS file or not (if not, probably a casa image...)
                bool itsIsFITSFile;

                /// The Cube of data, which contains the list of Detections.
                duchamp::Cube itsCube;

                /// The list of voxels encompassing detected sources, with fluxes.
                std::vector<PixelInfo::Voxel> itsVoxelList;

                /// The Gaussian Fitting parameter class
                sourcefitting::FittingParameters itsFitter;

                /// Shall we fit to the sources?
                bool itsFlagDoFit;

                /// Name of the summary file
                std::string itsSummaryFile;

                /// Name of the Karma annotation file with the fitted Gaussian components
                std::string itsFitAnnotationFile;

                /// The list of fits to the detected sources.
                std::vector<sourcefitting::RadioSource> itsSourceList;

                /// The list of sections corresponding to all workers' images (only used by the master).
                std::vector<duchamp::Section> itsSectionList;

                /// The definition of the subimage being used (only relevant for the workers)
                SubimageDef itsSubimageDef;

        };

    }
}
#endif
