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
#include <simulation/Simulator.h>

#include <APS/ParameterSet.h>

#include <boost/shared_ptr.hpp>

namespace conrad
{
	namespace synthesis
	{
		/// @brief Support for parallel simulation
		///
		/// @details A parset file provides a definition of all elements of the
		/// simulation. The worker number can be denoted by the string %w.
		/// Any definition may be redirected to another parset file using
		/// the definition key. For example, 
		/// @code
		/// Csimulator.dataset              =       ASKAP_spw_temporal%w.ms
		///
		///
		///  	Csimulator.sources.names                =       [10uJy]
		///  	Csimulator.sources.10uJy.direction       =       [12h30m00.000, -45.00.00.000, J2000]
		///  	Csimulator.sources.10uJy.model   =       10uJy.model
		///
		///  	Csimulator.antennas.definition =       ASKAP45.in
		///  	Csimulator.feeds.definition     =       ASKAP1feeds.in
		///  	Csimulator.spws.definition      =       ASKAPspws.in
		///
		///  	Csimulator.simulation.blockage             =       0.01
		///  	Csimulator.simulation.elevationlimit       =       8deg
		///  	Csimulator.simulation.autocorrwt           =       0.0
		///  	Csimulator.simulation.integrationtime      =       120s
		///  	Csimulator.simulation.usehourangles        =       True
		///  	Csimulator.simulation.referencetime       =       [2007Mar07, UTC]
		///
		///  	Csimulator.observe.number       =       1
		///  	Csimulator.observe.scan0        =       [10uJy, Temporal%w, -4h, 4h]
		/// @endcode
		/// The antennas parset file is:
		/// @code
		///		antennas.name            =       ASKAP45
		///		antennas.ASKAP45.location       =       [+117.471deg, -25.692deg, 192m, WGS84]
		///		antennas.ASKAP45.number         =       45
		///		antennas.ASKAP45.diameter       =       12m
		///		antennas.ASKAP45.mount          =       equatorial
		///		antennas.ASKAP45.antenna0       =      [-27.499744, 851.699585, 0.000000]
		///		antennas.ASKAP45.antenna1       =      [ 1251.443970,1132.437134,0.000000]
		///		antennas.ASKAP45.antenna2       =      [ -131.505112,2407.800293,0.000000]
		///		antennas.ASKAP45.antenna3       =      [ 1019.243896,1207.119873,0.000000]
		/// @endcode
		/// The feed parset file is:
		/// @code
		///		feeds.number         =       9
		///		feeds.spacing        =       1deg
		///		feeds.feed0          =       [-1, -1]
		///		feeds.feed1          =       [-1, 0]
		///		feeds.feed2          =       [-1, 1]
		///		feeds.feed3          =       [0, -1]
		///		feeds.feed4          =       [0, 0]
		///		feeds.feed5          =       [0, 1]
		///		feeds.feed6          =       [1, -1]
		///		feeds.feed7          =       [1, 0]
		///		feeds.feed8          =       [1, -1]
		/// @endcode
		/// The spws parset file is:
		/// @code
		///		spws.names      = [Continuum, Continuum0, Temporal, Temporal0, Temporal1, Temporal2, Temporal3, Spectral]
		///
		///		spws.Continuum  =[ 256, 1.420GHz, -1MHz, "XX XY YX YY"]
		///		spws.Continuum0 =[ 16, 1.420GHz, -1MHz, "XX XY YX YY"]
		///
		///		spws.Temporal   =[ 16, 1.420GHz, -16MHz, "XX YY"]
		///		spws.Temporal0  =[ 4, 1.420GHz, -16MHz, "XX YY"]
		///		spws.Temporal1  =[ 4, 1.356GHz, -16MHz, "XX YY"]
		///		spws.Temporal2  =[ 4, 1.292GHz, -16MHz, "XX YY"]
		///		spws.Temporal3  =[ 4, 1.228GHz, -16MHz, "XX YY"]
		///
		///		spws.Spectral   =[ 16384, 1.420GHz, -15.626kHz, "XX YY"]
		/// @endcode
		/// @ingroup parallel
		class SimParallel : public SynParallel
		{
			public:

				/// @brief Constructor from ParameterSet
				/// The command line inputs are needed solely for MPI - currently no
				/// application specific information is passed on the command line.
				/// @param argc Number of command line inputs
				/// @param argv Command line inputs
				/// @param parset ParameterSet for inputs
				SimParallel(int argc, const char** argv,
				    const LOFAR::ACC::APS::ParameterSet& parset);

				~SimParallel();

				/// @brief Initialize simulator
				/// @details The parset is used to construct the internal state. We could
				/// also support construction from a python dictionary (for example).
				void init();

				/// @brief Perform simulation, writing result to disk at the end
				/// @details The measurement set is constructed but not filled with data.
				/// At the end, the measurement set is written to disk.
				void simulate();

			private:
				/// Simulator
				boost::shared_ptr<Simulator> itsSim;

				/// MeasurementSet pointer - we need this to flush the MS to disk
				boost::shared_ptr<casa::MeasurementSet> itsMs;

				/// ParameterSet
				LOFAR::ACC::APS::ParameterSet itsParset;

				/// Read the telescope info from the parset specified in the main parset
				void readAntennas();

				/// Read the sources from the parset file (Worker only)
				void readSources();

				/// Read the models from the parset file (Master only)
				void readModels();

				/// Read the spectral window definitions
				void readSpws();

				/// Read the feed definitions
				void readFeeds();

				/// Read miscellaneous definitions for simulation
				void readSimulation();

				/// Predict data for current model
				/// @param ds Data set to predict for
				void predict(const std::string& ds);

		};

	}
}
#endif
