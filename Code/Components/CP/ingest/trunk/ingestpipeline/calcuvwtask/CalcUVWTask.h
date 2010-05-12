/// @file CalcUVWTask.h
///
/// @copyright (c) 2010 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_CALCUVWTASK_H
#define ASKAP_CP_CALCUVWTASK_H

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/datadef/VisChunk.h"
#include "ingestutils/AntennaPositions.h"

namespace askap {
namespace cp {

/// @brief UVW coordinate calculator task for the central processor ingest
/// pipeline.
///
/// This class encapsulates a task which runs as part of the central processor
/// ingest pipeline. This task calculates UVW coordinates for the visibilities
/// contained in a VisChunk.
///
/// The class requires antenna locations to be passed as part of the parameter
/// set passed to the constructor. The following is an example:
/// @verbatim
/// uvw.antennas.location = [+117.471deg, -25.692deg, 192m, WGS84]
/// uvw.antennas.names    = [A0, A1, A2, A3, A4, A5]
/// uvw.antenna.sscale    = 1.0
/// uvw.antennas.A0       = [-175.233429,  -1673.460938,  0.0000]
/// uvw.antennas.A1       = [261.119019,   -796.922119,   0.0000]
/// uvw.antennas.A2       = [-29.200520,   -744.432068,   0.0000]
/// uvw.antennas.A3       = [-289.355286,  -586.936035,   0.0000]
/// uvw.antennas.A4       = [-157.031570,  -815.570068,   0.0000]
/// uvw.antennas.A5       = [-521.311646,  -754.674927,   0.0000]
/// @endverbatim
///
/// @todo: Once a better way of managing configuration data (such as antenna
/// positions) is determined for ASKAPsoft, this class needs to be
/// modified accordingly.
///
/// This class implements the ITask interface which specified the process()
/// method. These "tasks" are treated polymorphically by the ingest pipeline.
/// Once data is sourced into the pipeline, the process() method is called
/// for each task (in a specific sequence), the VisChunk is read and/or modified
/// by each task.
class CalcUVWTask : public askap::cp::ITask {
    public:

        /// @breif Constructor.
        /// @param[in] parset the configuration parameter set.
        CalcUVWTask(const LOFAR::ParameterSet& parset);

        /// @brief Destructor.
        virtual ~CalcUVWTask();

        /// @brief Calculates UVW coordinates for each for in the
        /// specified VisChunk.
        ///
        /// @param[in,out] the instance of VisChunk for which UVW coordinates
        /// are to be calculated.
        virtual void process(VisChunk::ShPtr chunk);

    private:
        // Calculates UVW coordinates for the specified "row" in the "chunk"
        void calcForRow(VisChunk::ShPtr chunk, const casa::uInt row);

        // Parameter set
        const LOFAR::ParameterSet itsParset;

        // Antenna positions
        boost::scoped_ptr<AntennaPositions> itsAntennaPositions;
};

}
}

#endif
