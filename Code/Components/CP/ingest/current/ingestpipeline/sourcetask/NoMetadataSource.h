/// @file NoMetadataSource.h
///
/// @copyright (c) 2013 CSIRO
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

#ifndef ASKAP_CP_INGEST_NOMETADATASOURCE_H
#define ASKAP_CP_INGEST_NOMETADATASOURCE_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/system/error_code.hpp"
#include "boost/asio.hpp"
#include "Common/ParameterSet.h"
#include "cpcommon/VisDatagram.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/sourcetask/ISource.h"
#include "ingestpipeline/sourcetask/IVisSource.h"
#include "ingestpipeline/sourcetask/ScanManager.h"
#include "ingestpipeline/sourcetask/ChannelManager.h"
#include "configuration/Configuration.h"
#include "configuration/BaselineMap.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief Ingest pipeline source tasks. The NoMetadataSource task builds a VisChunk from
/// visibilities and configuration (in the parset) only, no TOs metadata is needed.
class NoMetadataSource : public ISource {
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
        NoMetadataSource(const LOFAR::ParameterSet& params,
                         const Configuration& config,
                         IVisSource::ShPtr visSource, int numTasks, int id);

        /// @brief Destructor.
        virtual ~NoMetadataSource();

        /// @brief Called to obtain the next VisChunk from the merged stream.
        /// @return a shared pointer to a VisChunk.
        virtual askap::cp::common::VisChunk::ShPtr next(void);

    private:

        askap::cp::common::VisChunk::ShPtr createVisChunk(const casa::uLong timestamp);

        void addVis(askap::cp::common::VisChunk::ShPtr chunk, const VisDatagram& vis,
                const casa::uInt nAntenna, const casa::uInt nBeams);

        void signalHandler(const boost::system::error_code& error,
                           int signalNumber);

        // Configuration
        const Configuration itsConfig;

        // The object that is the source of visibilities
        IVisSource::ShPtr itsVisSrc;

        // The total number of ingest pipeline tasks. Used to determine how many
        // visibilities this instance is responsible for receiving.
        int itsNumTasks;

        // The rank (identity amongst all ingest processes) of this process
        int itsId;

        // Pointers to the two constituent datatypes
        boost::shared_ptr<VisDatagram> itsVis;

        // Scan Manager
        ScanManager itsScanManager;

        // Channel Manager
        ChannelManager itsChannelManager;

        // Baseline Map
        const BaselineMap itsBaselineMap;

        // Interrupted by SIGTERM or SIGINT?
        bool itsInterrupted;

        // Boost io_service
        boost::asio::io_service itsIOService;

        // Interrupt signals
        boost::asio::signal_set itsSignals;

        // No support for assignment
        NoMetadataSource& operator=(const NoMetadataSource& rhs);

        // No support for copy constructor
        NoMetadataSource(const NoMetadataSource& src);
};

}
}
}

#endif
