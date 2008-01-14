/// @file
///
/// Provides generic methods for parallel algorithms
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_CP_DUCHAMPPARALLEL_H_
#define CONRAD_CP_DUCHAMPPARALLEL_H_

#include <conradparallel/ConradParallel.h>

#include <APS/ParameterSet.h>

#include <duchamp/Cubes/cubes.hh>

namespace conrad
{
  namespace analysis
  {
    /// @brief Support for parallel algorithms 
    ///
    /// @details Support for parallel applications in the area.
    /// An application is derived from this abstract base. The model used is that the
    /// application has many workers and one master, running in separate MPI processes
    /// or in one single thread. The master is the master so the number of processes
    /// is one more than the number of workers. 
    ///
    /// If the number of nodes is 1 then everything occurs in the same process with
    /// no overall for transmission of model.
    ///
    /// @ingroup parallel
    class DuchampParallel : public conrad::cp::ConradParallel
    {
  public:

      /// @brief Constructor 
      /// @details The command line inputs are needed solely for MPI - currently no
      /// application specific information is passed on the command line.
      /// @param argc Number of command line inputs
      /// @param argv Command line inputs
      DuchampParallel(int argc, const char** argv, const LOFAR::ACC::APS::ParameterSet& parset);

      // Read in the data from the image file (on the workers)
      void readData();
      
      // Condense the lists (on the master)
      void condenseLists();
      
      // Find the lists (on the workers)
      void findLists();

      // Print out the resulting source list (on the master)
      void printResults();
      
      // Find the statistics (on the workers)
      void findStatistics();

      // Print the statistics (on the master)
      void printStatistics();


      // Better ones:
      // Find the mean (on the workers)
      void findMeans();
      // Find the RMS (on the workers)
      void findRMSs();
      // Combine and print the mean (on the master)
      void combineMeans();
      // Send the overall mean to the workers (on the master)
      void broadcastMean();
      // Combine and print the RMS (on the master)
      void combineRMSs();

  protected:

      std::string itsImage;

      duchamp::Cube itsCube;

    };

  }
}
#endif
