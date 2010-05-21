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

#ifndef ASKAP_CP_MERGEDSOURCE_H
#define ASKAP_CP_MERGEDSOURCE_H

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

class MergedSource {
    public:
        MergedSource(IMetadataSource::ShPtr metadataSource,
                     IVisSource::ShPtr visSource);
        ~MergedSource();

        // Blocking
        VisChunk::ShPtr next(void);

    private:

        VisChunk::ShPtr createVisChunk(const TosMetadata& metadata);
        void addVis(VisChunk::ShPtr chunk, const VisDatagram& vis);
        void doFlagging(VisChunk::ShPtr chunk, const TosMetadata& metadata);
        void doFlaggingSample(VisChunk::ShPtr chunk,
                              const TosMetadata& metadata,
                              const unsigned int row,
                              const unsigned int chan,
                              const unsigned int pol);

        unsigned int fineToCoarseChannel(const unsigned int& fineChannel);

        IMetadataSource::ShPtr itsMetadataSrc;
        IVisSource::ShPtr itsVisSrc;

        // Pointers to the two constituent datatypes
        boost::shared_ptr<TosMetadata> itsMetadata;
        boost::shared_ptr<VisDatagram> itsVis;

};

}
}

#endif
