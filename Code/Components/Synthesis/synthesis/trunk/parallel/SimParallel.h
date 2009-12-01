/// @file SimParallel.h
///
/// SimParallel: Support for parallel simulation
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
#ifndef ASKAP_SYNTHESIS_SIMPARALLEL_H_
#define ASKAP_SYNTHESIS_SIMPARALLEL_H_

#include <boost/shared_ptr.hpp>
#include <Common/ParameterSet.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <dataaccess/SharedIter.h>
#include <measurementequation/IMeasurementEquation.h>
#include <askapparallel/AskapParallel.h>
#include <parallel/SynParallel.h>
#include <simulation/Simulator.h>

namespace askap
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
                SimParallel(askap::mwbase::AskapParallel& comms,
                        const LOFAR::ParameterSet& parset);

                ~SimParallel();

                /// @brief Initialize simulator
                /// @details The parset is used to construct the internal state. We could
                /// also support construction from a python dictionary (for example).
                void init();

                /// @brief Perform simulation, writing result to disk at the end
                /// @details The measurement set is constructed but not filled with data.
                /// At the end, the measurement set is written to disk.
                void simulate();

            protected:
                /// @brief a helper method to add up an equation
                /// @details Some times it is necessary to replace a measurement equation
                /// with a sum of two equations. Typical use cases are adding noise to
                /// the visibility data and simulating using a composite model containing
                /// both components and images. This method replaces the input equation
                /// with the sum of the input equation and some other equation also passed
                /// as a parameter. It takes care of equation types and instantiates 
                /// adapters if necessary.
                /// @param[in] equation a non-const reference to the shared pointer holding
                /// an equation to update
                /// @param[in] other a const reference to the shared pointer holding
                /// an equation to be added
                /// @param[in] it iterator over the dataset (this is a legacy of the current
                /// design of the imaging code, when equation requires an iterator. It should 
                /// get away at some stage)
                /// @note This method can be moved somewhere else, as it may be needed in
                /// some other places as well
                static void addEquation(boost::shared_ptr<scimath::Equation> &equation,
                        const boost::shared_ptr<IMeasurementEquation const> &other, 
                        const IDataSharedIter &it);

                /// @brief a helper method to corrupt the data (opposite to calibration)
                /// @details Applying gains require different operations depending on
                /// the type of the measurement equation (accessor-based or iterator-based).
                /// It is encapsulated in this method. The method accesses itsParset to
                /// extract the information about calibration model.
                /// @param[in] equation a non-const reference to the shared pointer holding
                /// an equation to update
                /// @param[in] it iterator over the dataset (this is a legacy of the current
                /// design of the imaging code, when equation requires an iterator. It should 
                /// get away at some stage)
                void corruptEquation(boost::shared_ptr<scimath::Equation> &equation,
                        const IDataSharedIter &it);

            private:
                /// Simulator
                boost::shared_ptr<Simulator> itsSim;

                /// MeasurementSet pointer - we need this to flush the MS to disk
                boost::shared_ptr<casa::MeasurementSet> itsMs;

                /// ParameterSet
                LOFAR::ParameterSet itsParset;

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
