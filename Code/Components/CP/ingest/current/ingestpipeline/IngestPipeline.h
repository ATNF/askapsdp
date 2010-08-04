/// @file IngestPipeline.h
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

#ifndef ASKAP_CP_INGESTPIPELINE_H
#define ASKAP_CP_INGESTPIPELINE_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "boost/scoped_ptr.hpp"

// Local package includes
#include "ingestpipeline/datadef/VisChunk.h"
#include "ingestpipeline/sourcetask/MergedSource.h"
#include "ingestpipeline/ITask.h"

namespace askap {
namespace cp {

class IngestPipeline {
    public:
        /// @brief Constructor.
        IngestPipeline(const LOFAR::ParameterSet& parset);

        /// @brief Destructor.
        ~IngestPipeline();

        /// @brief Start running the pipeline.
        /// This is a blocking call, the IngestPipeline runs using the calling
        /// thread. It will return only when the observation has completed
        /// or when abort() is called.
        void start(void);

        /// Brief Abort the pipeline as soon as possible.
        /// Calling this method instructs the pipeline to finish up as soon
        /// as possible, however this method returns immediatly and does not
        /// wait.
        void abort(void);

    private:
        void ingest(void);

        bool ingestOne(void);

        void createSource(void);

        const LOFAR::ParameterSet itsParset;

        bool itsRunning;

        boost::scoped_ptr< MergedSource > itsSource;

        std::vector<ITask::ShPtr> itsTasks;

        // A count of how many correlator integrations have been
        // processed.
        unsigned long itsIntegrationsCount;

        // Number of correlator integrations expected. When itsIntegrationsCount
        // equals this number the pipeline will be stopped and start will
        // return.
        unsigned long itsIntegrationsExpected;
};

};
};

#endif
