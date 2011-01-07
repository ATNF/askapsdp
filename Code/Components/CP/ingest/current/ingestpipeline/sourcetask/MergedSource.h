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
#include "boost/shared_ptr.hpp"
#include "cpcommon/TosMetadata.h"
#include "cpcommon/VisDatagram.h"

// Local package includes
#include "ingestpipeline/sourcetask/IVisSource.h"
#include "ingestpipeline/sourcetask/IMetadataSource.h"
#include "ingestpipeline/datadef/VisChunk.h"

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
        /// @param[in] metadataSource   Instance of a IMetadataSource from which the TOS
        ///                             TOS metadata will be sourced.
        /// @param[in] visSource    Instance of a IVisSource from which the visibilities
        ///                         will be sourced.
        /// @param[in] numTasks     Total number of ingest pipeline tasks. This enables
        ///                         the merged source to determine how many visibilities
        ///                         it is responsible for receiving.
        MergedSource(IMetadataSource::ShPtr metadataSource,
                     IVisSource::ShPtr visSource, int numTasks);

        /// @brief Destructor.
        ~MergedSource();

        /// @brief Called to obtain the next VisChunk from the merged stream.
        /// @return a shared pointer to a VisChunk.
        VisChunk::ShPtr next(void);

    private:

        VisChunk::ShPtr createVisChunk(const TosMetadata& metadata);

        void addVis(VisChunk::ShPtr chunk, const VisDatagram& vis,
                const casa::uInt nAntenna, const casa::uInt nBeams);

        void doFlagging(VisChunk::ShPtr chunk, const TosMetadata& metadata);

        void doFlaggingSample(VisChunk::ShPtr chunk,
                              const TosMetadata& metadata,
                              const unsigned int row,
                              const unsigned int chan,
                              const unsigned int pol);

        unsigned int fineToCoarseChannel(const unsigned int& fineChannel);

        // The two sources which will be merged. These two objects provide
        // the input stream.
        IMetadataSource::ShPtr itsMetadataSrc;
        IVisSource::ShPtr itsVisSrc;

        // The total number of ingest pipeline tasks. Used to determine how many
        // visibilities this instance is responsible for recieving.
        int itsNumTasks;

        // Pointers to the two constituent datatypes
        boost::shared_ptr<TosMetadata> itsMetadata;
        boost::shared_ptr<VisDatagram> itsVis;

        // No support for assignment
        MergedSource& operator=(const MergedSource& rhs);

        // No support for copy constructor
        MergedSource(const MergedSource& src);
};

}
}
}

#endif
