/// @file FringeRotationTask.h
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef ASKAP_CP_INGEST_FRINGEROTATIONTASK_H
#define ASKAP_CP_INGEST_FRINGEROTATIONTASK_H

// std includes
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"

// casa includes
#include <casa/Arrays/Matrix.h>

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/calcuvwtask/CalcUVWTask.h"
#include "ingestpipeline/phasetracktask/IFrtApproach.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {


/// @brief this is a generalised task for fringe rotation
/// @details There are a number of approaches to fringe rotation with different limitations.
/// Unlike PhaseTrackTask which does essentially only phase tracking (with limited experiments
/// on delay tracking), this task is intended for more accurate approaches which talk to hardware
/// (indirectly) and synchronise application of delays and rates with residual corrections in software.
/// The actual work takes place in the implementations of the IFrtApproach interface. This class
/// implements the actual delay model and the task interface.
///
/// @note For simplicity, this class is derived from UVW calculator class.
class FringeRotationTask : public askap::cp::ingest::CalcUVWTask {
    public:

        /// @brief Constructor.
        /// @param[in] parset the configuration parameter set.
        /// @param[in] config configuration
        FringeRotationTask(const LOFAR::ParameterSet& parset,
                       const Configuration& config);

        /// @brief process one VisChunk 
        /// @details Perform fringe tracking, correct residual effects on visibilities in the specified VisChunk.
        ///
        /// @param[in,out] chunk  the instance of VisChunk for which the
        ///                       phase factors will be applied.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

    protected:
        /// @brief factory method for the fringe rotation approach classes
        /// @details This class is used to create implementations of 
        /// IFrtApproach interface based on the parset. These classes do
        /// actual work on application of delays and rates
        /// @param[in] parset the configuration parameter set.
        /// @param[in] config configuration
        static IFrtApproach::ShPtr fringeRotationMethod(const LOFAR::ParameterSet& parset, 
               const Configuration &config);
        
        /// @brief helper method to find dish pointing for a given antenna index
        /// @param[in] chunk the instance of VisChunk to search through
        /// @param[in] ant antenna index
        /// @return dish pointing
        casa::MVDirection dishPointing(const askap::cp::common::VisChunk::ShPtr &chunk, const casa::uInt ant) const;

    private:
        /// @brief configuration (need scan information)
        Configuration itsConfig;

        /// @brief fixed delay component in ns
        ///
        /// @details This attribute constrols whether fixed delays are added. The values (one
        /// delay per antenna) are simply added to the geometric delay if tracked or applied
        /// as they are. If antenna ID exceeds the size of the vector, the delay is assumed to
        /// be zero. Zero length means no application of the fixed delay.
        std::vector<double> itsFixedDelays;

        /// @details actual class applying calculated delays and rates
        IFrtApproach::ShPtr   itsFrtMethod;
}; // FringeRotationTask class

} // ingest
} // cp
} // askap

#endif // #ifndef ASKAP_CP_INGEST_FRINGEROTATION_H

