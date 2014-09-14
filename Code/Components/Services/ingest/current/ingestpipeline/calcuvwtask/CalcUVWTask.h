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

#ifndef ASKAP_CP_INGEST_CALCUVWTASK_H
#define ASKAP_CP_INGEST_CALCUVWTASK_H

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"
#include "scimath/Mathematics/RigidVector.h"
#include "casa/Arrays/Vector.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {

/// @brief UVW coordinate calculator task for the central processor ingest
/// pipeline.
///
/// This class encapsulates a task which runs as part of the central processor
/// ingest pipeline. This task calculates UVW coordinates for the visibilities
/// contained in a VisChunk.
///
/// This class implements the ITask interface which specified the process()
/// method. These "tasks" are treated polymorphically by the ingest pipeline.
/// Once data is sourced into the pipeline, the process() method is called
/// for each task (in a specific sequence), the VisChunk is read and/or modified
/// by each task.
class CalcUVWTask : public askap::cp::ingest::ITask {
    public:

        /// @brief Constructor.
        /// @param[in] parset the configuration parameter set.
        CalcUVWTask(const LOFAR::ParameterSet& parset,
                const Configuration& config);

        /// @brief Destructor.
        virtual ~CalcUVWTask();

        /// @brief Calculates UVW coordinates for each for in the
        /// specified VisChunk.
        ///
        /// @param[in,out] chunk  the instance of VisChunk for which UVW
        ///                       coordinates are to be calculated.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);
  
    protected:

        /// @brief obtain ITRF coordinates of a given antenna
        /// @details
        /// @param[in] ant antenna index
        /// @return 3-element vector with X,Y and Z
        casa::Vector<double> antXYZ(const casa::uInt ant) const;

        /// @brief obtain maximum number of antennas
        /// @return maximum number of antennas
        inline casa::uInt nAntennas() const { return itsAntXYZ.ncolumn(); }

        /// @brief obtain maximum number of beams
        /// @return maximum number of beams
        inline casa::uInt nBeams() const { return itsBeamOffset.nelements(); }

        /// @brief obtain phase centre for a given beam
        /// @details This method encapsulates common operations to obtain the direction
        /// of the phase centre for an (off-axis) beam by shifting dish pointing centre
        /// @param[in] dishPointing pointing centre for the whole dish
        /// @param[in] beam beam index to work 
        /// @return direction measure for the phase centre
        casa::MDirection phaseCentre(const casa::MDirection &dishPointing,
                                     const casa::uInt beam) const;

        /// @brief obtain gast for the given epoch
        /// @param[in] epoch UTC epoch to convert to GAST
        /// @return gast in radians modulo 2pi
        static double calcGAST(const casa::MVEpoch &epoch);
 
    private:
        // Calculates UVW coordinates for the specified "row" in the "chunk"
        void calcForRow(askap::cp::common::VisChunk::ShPtr chunk, const casa::uInt row);

        // Populates the antenna Position Matrix
        void createPositionMatrix(const Configuration& config);

        // Populates the itsBeamOffset vector
        void setupBeamOffsets(const Configuration& config);

        // A matrix containing antenna positions.
        // The antenna positions. Size is 3 (x, y & z) rows by nAntenna columns.
        // Rows are x, y, z and columns are indexed by antenna id.
        casa::Matrix<double> itsAntXYZ;

        // A vector with one element per beam. Each element then is a
        // two element vector containing the x and y offsets at index
        // 0 and 1 respectivly
        casa::Vector< casa::RigidVector<double, 2> > itsBeamOffset;
};

}
}
}

#endif
