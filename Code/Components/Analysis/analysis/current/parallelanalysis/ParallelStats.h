/// @file
///
/// Obtaining image statistics through distributed processing
///
/// @copyright (c) 2011 CSIRO
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
#ifndef ASKAP_ANALYSIS_PARALLELSTATS_H_
#define ASKAP_ANALYSIS_PARALLELSTATS_H_

#include <askapparallel/AskapParallel.h>
#include <duchamp/Cubes/cubes.hh>

namespace askap {

namespace analysis {

class ParallelStats {
    public:
        ParallelStats(askap::askapparallel::AskapParallel& comms,
                      duchamp::Cube *cube);
        virtual ~ParallelStats() {};

        /// @brief Find the statistics by looking at the distributed data.
        void findDistributedStats();

        /// @brief Find the mean (on the workers)
        /// @details This finds the mean or median (according to the
        /// flagRobustStats parameter) of the worker's image/cube,
        /// then sends that value to the master via LOFAR Blobs.
        void findMeans();

        /// @brief Find the STDDEV (on the workers) @details This
        /// finds the stddev or the median absolute deviation from the
        /// median (MADFM) (dictated by the flagRobustStats parameter)
        /// of the worker's image/cube, then sends that value to the
        /// master via LOFAR Blobs. To calculate the stddev/MADFM, the
        /// mean of the full dataset must be read from the master
        /// (again passed via LOFAR Blobs). The calculation uses the
        /// findSpread() function.
        void findStddevs();

        /// @brief Combine and print the mean (on the master) @details
        /// The master reads the mean/median values from each of the
        /// workers, and combines them to form the mean/median of the
        /// full dataset. Note that if the median of the workers data
        /// has been provided, the values are treated as estimates of
        /// the mean, and are combined as if they were means (ie. the
        /// overall value is the weighted (by size) average of the
        /// means/medians of the individual images). The value is
        /// stored in the StatsContainer in itsCube.
        void combineMeans();

        /// @brief Send the overall mean to the workers (on the master)
        /// @details The mean/median value of the full dataset is sent
        /// via LOFAR Blobs to the workers.
        void broadcastMean();

        /// @brief Combine and print the STDDEV (on the master)
        /// @details The master reads the stddev/MADFM values from
        /// each of the workers, and combines them to produce an
        /// estimate of the stddev for the full cube. Again, if MADFM
        /// values have been calculated on the workers, they are
        /// treated as estimates of the stddev and are combined as if
        /// they are stddev values. The overall value is stored in the
        /// StatsContainer in itsCube
        void combineStddevs();

        /// @brief Printing cube stats to the log
        void printStats();

    protected:
        askap::askapparallel::AskapParallel *itsComms;
        duchamp::Cube *itsCube;


};

}

}


#endif
