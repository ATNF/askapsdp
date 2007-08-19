/// @file
///
/// SimParallel: Support for parallel simulation
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_SIMPARALLEL_H_
#define CONRAD_SYNTHESIS_SIMPARALLEL_H_

#include <parallel/SynParallel.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/NewMSSimulator.h>

#include <APS/ParameterSet.h>

#include <boost/shared_ptr.hpp>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Support for parallel simulation
    ///
    /// @details A parset file provides a definition of all elements of the
  	/// simulation. 
    /// @ingroup parallel
    class SimParallel: public SynParallel
    {
      public:

        /// @brief Constructor from ParameterSet
      	/// @details The parset is used to construct the internal state. We could
      	/// also support construction from a python dictionary (for example).
      	/// The command line inputs are needed solely for MPI - currently no
      	/// application specific information is passed on the command line.
      	/// @param argc Number of command line inputs
      	/// @param argv Command line inputs
      	/// @param parset ParameterSet for inputs
      	SimParallel(int argc, const char** argv, const LOFAR::ACC::APS::ParameterSet& parset);
      	
      	~SimParallel();
      	
      	/// @brief Perform simulation, writing result to disk at the end
      	/// @details The measurement set is constructed but not filled with data.
      	/// At the end, the measurement set is written to disk.
      	void simulate();

      private:
      	/// casacore Simulator
      	boost::shared_ptr<casa::NewMSSimulator> itsSim;
      	
      	/// MeasurementSet pointer - we need this to flush the MS to disk
      	boost::shared_ptr<casa::MeasurementSet> itsMs;

      	/// ParameterSet
      	LOFAR::ACC::APS::ParameterSet itsParset;
      	
      	/// Read the telescope info from the parset specified in the main parset
      	void readAntennas();
      	
      	/// Read the sources from the parset file
      	void readSources();
      	
      	/// Read the spectral window definitions
      	void readSpws();
      	
      	/// Read the feed definitions
      	void readFeeds();
      	
      	/// Read miscellaneous definitions for simulation
      	void readSimulation();
    		
    };

  }
}
#endif
