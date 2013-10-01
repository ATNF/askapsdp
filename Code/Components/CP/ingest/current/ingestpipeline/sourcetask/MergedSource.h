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

// ASKAPsoft includes
#include "askap/IndexConverter.h"
#include "boost/shared_ptr.hpp"
#include "boost/system/error_code.hpp"
#include "boost/asio.hpp"
#include "Common/ParameterSet.h"
#include "cpcommon/TosMetadata.h"
#include "cpcommon/VisDatagram.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/sourcetask/ISource.h"
#include "ingestpipeline/sourcetask/IVisSource.h"
#include "ingestpipeline/sourcetask/IMetadataSource.h"
#include "ingestpipeline/sourcetask/ScanManager.h"
#include "ingestpipeline/sourcetask/ChannelManager.h"
#include "configuration/Configuration.h"
#include "configuration/BaselineMap.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief Ingest pipeline source tasks. The MergedSource task merges the TOS
/// metadata stream and the visibility stream creating a VISChunk object for
/// each correlator integration.
class MergedSource : public ISource {
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
        virtual ~MergedSource();

        /// @brief Called to obtain the next VisChunk from the merged stream.
        /// @return a shared pointer to a VisChunk.
        virtual askap::cp::common::VisChunk::ShPtr next(void);

    private:

        askap::cp::common::VisChunk::ShPtr createVisChunk(const TosMetadata& metadata);

        bool addVis(askap::cp::common::VisChunk::ShPtr chunk, const VisDatagram& vis,
                const casa::uInt nAntenna);

        void doFlagging(askap::cp::common::VisChunk::ShPtr chunk, const TosMetadata& metadata);

        void doFlaggingSample(askap::cp::common::VisChunk::ShPtr chunk,
                              const TosMetadata& metadata,
                              const unsigned int row,
                              const unsigned int chan,
                              const unsigned int pol);

        /// Handled the receipt of signals to "interrupt" the process
        void signalHandler(const boost::system::error_code& error,
                           int signalNumber);

        void parseBeamMap(const LOFAR::ParameterSet& params);

        // Checks if a signal has been received requesting an interrupt.
        // If such a signal has been received, thorows an InterruptedException.
        void checkInterruptSignal();

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

        // Scan Manager
        ScanManager itsScanManager;

        // Channel Manager
        ChannelManager itsChannelManager;

        // Baseline Map
        const BaselineMap itsBaselineMap;

        // Interrupted by SIGTERM, SIGINT or SIGUSR1?
        bool itsInterrupted;

        // Boost io_service
        boost::asio::io_service itsIOService;

        // Interrupt signals
        boost::asio::signal_set itsSignals;

        /// @brief beam id map
        /// @details It is possible to filter the beams received by this source and map the
        /// indices. This map provides translation (by default, any index is passed as is)
        utility::IndexConverter itsBeamIDMap;

        /// @brief largest supported number of beams
        /// @details The space is reserved for the given number of beams (set via the parset).
        /// This value is always less than or equal to the number of beams specified via the
        /// configuration (the latter is the default). The visibility cube is resized to match
        /// this parameter (allowing to drop unnecessary beams if used together with itsBeamIDMap)
        /// @note it is 0 by default, which triggers the constructor to set it equal to the
        /// configuration (i.e. to write everything)
        casa::uInt itsMaxNBeams;

        /// @brief number of beams to expect in the data stream
        /// @details A larger number of beams can be received from the datastream than stored into MS.
        /// To avoid unnecessary bloat of the MS size, only itsMaxNBeams beams are stored. This field
        /// controls the data stream unpacking.
        /// @note it is 0 by default, which triggers the constructor to set it equal to the
        /// configuration (i.e. to write everything)
        casa::uInt itsBeamsToReceive;

        // No support for assignment
        MergedSource& operator=(const MergedSource& rhs);

        // No support for copy constructor
        MergedSource(const MergedSource& src);
};

}
}
}

#endif
