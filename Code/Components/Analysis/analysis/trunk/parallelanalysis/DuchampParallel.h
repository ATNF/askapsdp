/// @file
///
/// Provides generic methods for parallel algorithms
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef ASKAP_ANALYSIS_DUCHAMPPARALLEL_H_
#define ASKAP_ANALYSIS_DUCHAMPPARALLEL_H_

#include <sourcefitting/RadioSource.h>

#include <askapparallel/AskapParallel.h>

#include <APS/ParameterSet.h>

#include <duchamp/Cubes/cubes.hh>
#include <duchamp/PixelMap/Voxel.hh>

#include <vector>
#include <string>

namespace askap
{
  namespace analysis
  {
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
    class DuchampParallel : public askap::cp::AskapParallel
    {
    public:

      /// @brief Constructor 
      /// @details The command line inputs are needed solely for MPI - currently no
      /// application specific information is passed on the command line.
      /// @param argc Number of command line inputs
      /// @param argv Command line inputs
      /// @param parset The parameter set to read Duchamp and other parameters from.
      DuchampParallel(int argc, const char** argv, const LOFAR::ACC::APS::ParameterSet& parset);

      /// @brief Read in the data from the image file (on the workers)
      void readData();
      
      /// @brief Condense the lists (on the master)
      void condenseLists();
      
      /// @brief Find the lists (on the workers)
      void findLists();

      /// @brief Sort out the fluxes for all detected objects (on the master)
      void calcFluxes();

      /// @brief Print out the resulting source list (on the master)
      void printResults();

      /// @brief Fit the detected sources (on the master)
      void fitSources();

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

    protected:

      /// The name of the file containing the image data.
      std::string itsImage;

      /// The Cube of data, which contains the list of Detections.
      duchamp::Cube itsCube;

      /// The list of voxels encompassing detected sources, with fluxes.
      std::vector<PixelInfo::Voxel> itsVoxelList;

      /// Shall we fit to the sources?
      bool itsFlagDoFit;

      /// Name of the Karma annotation file with the fitted Gaussian components
      std::string itsFitAnnotationFile;
      
      /// The list of fits to the detected sources.
      std::vector<askap::sourcefitting::RadioSource> itsSourceList;

      /// The list of sections corresponding to all workers' images (only used by the master).
      std::vector<duchamp::Section> itsSectionList;

    };

  }
}
#endif
