/// @file MergedSource.h
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

#ifndef ASKAP_CP_INGEST_MERGEDSOURCE_H
#define ASKAP_CP_INGEST_MERGEDSOURCE_H

// System includes
#include <map>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "Common/ParameterSet.h"
#include "cpcommon/TosMetadata.h"
#include "cpcommon/VisDatagram.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/sourcetask/IVisSource.h"
#include "ingestpipeline/sourcetask/IMetadataSource.h"
#include "ingestpipeline/sourcetask/ScanManager.h"
#include "configuration/Configuration.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief Ingest pipeline source tasks. The MergedSource task merges the TOS
/// metadata stream and the visibility stream creating a VISChunk object for
/// each correlator integration.
class MergedSource {
    public:
        /// @brief Constructor.
        ///
        /// @param[in] params           Parameters specific to this task
        /// @param[in] config           Configuration
        /// @param[in] metadataSource   Instance of a IMetadataSource from which the TOS
        ///                             TOS metadata will be sourced.
        /// @param[in] visSource    Instance of a IVisSource from which the visibilities
        ///                         will be sourced.
        /// @param[in] numTasks     Total number of ingest pipeline tasks. This enables
        ///                         the merged source to determine how many visibilities
        ///                         it is responsible for receiving.
        MergedSource(const LOFAR::ParameterSet& params,
                     const Configuration& config,
                     IMetadataSource::ShPtr metadataSource,
                     IVisSource::ShPtr visSource, int numTasks, int id);

        /// @brief Destructor.
        ~MergedSource();

        /// @brief Called to obtain the next VisChunk from the merged stream.
        /// @return a shared pointer to a VisChunk.
        askap::cp::common::VisChunk::ShPtr next(void);

    private:

        askap::cp::common::VisChunk::ShPtr createVisChunk(const TosMetadata& metadata);

        void addVis(askap::cp::common::VisChunk::ShPtr chunk, const VisDatagram& vis,
                const casa::uInt nAntenna, const casa::uInt nBeams);

        void doFlagging(askap::cp::common::VisChunk::ShPtr chunk, const TosMetadata& metadata);

        void doFlaggingSample(askap::cp::common::VisChunk::ShPtr chunk,
                              const TosMetadata& metadata,
                              const unsigned int row,
                              const unsigned int chan,
                              const unsigned int pol);

        // Convert from find channels to coarse
        unsigned int fineToCoarseChannel(const unsigned int& fineChannel);

        // Initialise the ingest process to #channels map
        std::map<int, unsigned int> initChannelMappings(const LOFAR::ParameterSet& params);

        // Returns the number of channels handled by this ingest process
        unsigned int nChannelsHandled();

        // Configuration
        const Configuration itsConfig;

        // The object that is the source of telescope metadata
        IMetadataSource::ShPtr itsMetadataSrc;
        
        // The object that is the source of visibilities
        IVisSource::ShPtr itsVisSrc;

        // The total number of ingest pipeline tasks. Used to determine how many
        // visibilities this instance is responsible for receiving.
        int itsNumTasks;

        // The rank (identity amongst all ingest processes) of this process
        int itsId;

        // Pointers to the two constituent datatypes
        boost::shared_ptr<TosMetadata> itsMetadata;
        boost::shared_ptr<VisDatagram> itsVis;

        // Maps rank to number of channels
        const std::map<int, unsigned int> itsChansPerRank;

        // Scan Manager
        ScanManager itsScanManager;

        // No support for assignment
        MergedSource& operator=(const MergedSource& rhs);

        // No support for copy constructor
        MergedSource(const MergedSource& src);
};

}
}
}

#endif
