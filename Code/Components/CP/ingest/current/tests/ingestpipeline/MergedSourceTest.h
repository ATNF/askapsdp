/// @file MergedSourceTest.h
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

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include <cmath>
#include <stdint.h>
#include "boost/shared_ptr.hpp"
#include "Common/ParameterSet.h"
#include "ingestpipeline/sourcetask/test/MockMetadataSource.h"
#include "ingestpipeline/sourcetask/test/MockVisSource.h"
#include "cpcommon/VisDatagram.h"
#include "cpcommon/TosMetadata.h"
#include "cpcommon/TosMetadataAntenna.h"
#include "cpcommon/VisChunk.h"
#include "measures/Measures.h"
#include "ConfigurationHelper.h"

// Classes to test
#include "ingestpipeline/sourcetask/MergedSource.h"

using askap::cp::common::VisChunk;

namespace askap {
namespace cp {
namespace ingest {

class MergedSourceTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(MergedSourceTest);
        CPPUNIT_TEST(testMockMetadataSource);
        CPPUNIT_TEST(testMockVisSource);
        CPPUNIT_TEST(testSingle);
        CPPUNIT_TEST_SUITE_END();

    public:

        void setUp() {
            itsMetadataSrc.reset(new MockMetadataSource);
            itsVisSrc.reset(new MockVisSource);

            LOFAR::ParameterSet params;
            std::ostringstream ss;
            ss << N_CHANNELS_PER_SLICE;
            params.add("n_channels.0", ss.str());
            const Configuration config = ConfigurationHelper::createDummyConfig();
            itsInstance.reset(new MergedSource(params, config, itsMetadataSrc, itsVisSrc, 1, 0));
        }

        void tearDown() {
            itsInstance.reset();
            itsVisSrc.reset();
            itsMetadataSrc.reset();
        }

        // Test the MockMetadataSource before using it
        void testMockMetadataSource() {
            const int64_t time = 1234;
            boost::shared_ptr<TosMetadata> md(new TosMetadata());
            md->time(time);
            itsMetadataSrc->add(md);
            CPPUNIT_ASSERT(itsMetadataSrc->next() == md);
        };

        // Test the MockVisSource before using it
        void testMockVisSource() {
            const int64_t time = 1234;
            boost::shared_ptr<VisDatagram> vis(new VisDatagram);
            vis->timestamp = time;
            itsVisSrc->add(vis);
            CPPUNIT_ASSERT(itsVisSrc->next() == vis);
        };

        void testSingle() {
            const Configuration config = ConfigurationHelper::createDummyConfig();
            const uint64_t starttime = 1000000; // One second after epoch
            const uint64_t period = 5 * 1000 * 1000;
            const uint32_t nCorr = 4;

            // Create a mock metadata object and program it, then
            // add to the MockMetadataSource
            TosMetadata metadata;
            metadata.time(starttime);
            metadata.scanId(0);
            metadata.flagged(false);
            metadata.corrMode("standard");

            // antenna_names
            for (uint32_t i = 0; i < config.antennas().size(); ++i) {
                TosMetadataAntenna ant(config.antennas()[i].name());
                ant.onSource(true);
                ant.flagged(false);
                metadata.addAntenna(ant);
            }

            // Make a copy of the metadata and add it to the mock
            // Metadata source
            boost::shared_ptr<TosMetadata> copy(new TosMetadata(metadata));
            itsMetadataSrc->add(copy);

            // Populate a VisDatagram to match the metadata
            askap::cp::VisDatagram vis;
            vis.version = VISPAYLOAD_VERSION;
            vis.slice = 0;
            vis.baselineid = 1;
            vis.beamid = 1;
            vis.timestamp = starttime;

            boost::shared_ptr<VisDatagram> copy1(new VisDatagram(vis));
            itsVisSrc->add(copy1);

            vis.timestamp = starttime + period;
            boost::shared_ptr<VisDatagram> copy2(new VisDatagram(vis));
            itsVisSrc->add(copy2);

            // Get the first VisChunk instance
            VisChunk::ShPtr chunk(itsInstance->next());
            CPPUNIT_ASSERT(chunk.get());

            // Ensure the timestamp represents the integration midpoint.
            // Note the TosMetadata timestamp is the integration start (in
            // microseconds) while the VisChunk timestamp is the integration
            // midpoint (in seconds). The later is that way because the
            // measurement set specification used integration midpoint in
            // seconds.
            //const double midpoint = 3.5;
            //casa::Quantity chunkMidpoint = chunk->time().getTime();
            //CPPUNIT_ASSERT_DOUBLES_EQUAL(midpoint, chunkMidpoint.getValue("s"), 1.0E-10);

            // Ensure other metadata is as expected
            CPPUNIT_ASSERT_EQUAL(1 * N_CHANNELS_PER_SLICE, chunk->nChannel());
            CPPUNIT_ASSERT_EQUAL(nCorr, chunk->nPol());
            const uint32_t nBaselines = config.bmap().size();
            const uint32_t nBeam = config.feed().nFeeds();
            CPPUNIT_ASSERT_EQUAL(nBaselines * nBeam, chunk->nRow());

            // Ensure the visibilities that were supplied (most were not)
            // are not flagged, and that the rest are flagged

            // First calculate the channel range that was set and the antenna pair
            const uint32_t startChan = vis.slice * N_CHANNELS_PER_SLICE; //inclusive
            const uint32_t endChan = (vis.slice + 1) * N_CHANNELS_PER_SLICE; //exclusive
            const int32_t ant1 = config.bmap().idToAntenna1(vis.baselineid);
            CPPUNIT_ASSERT(ant1 != -1); // -1 represents an invalid mapping
            const int32_t ant2 = config.bmap().idToAntenna2(vis.baselineid);
            CPPUNIT_ASSERT(ant2 != -1);

            for (uint32_t row = 0; row < chunk->nRow(); ++row) {
                for (uint32_t chan = 0; chan < chunk->nChannel(); ++chan) {
                    for (uint32_t pol = 0; pol < chunk->nPol(); ++pol) {
                        if (chan >= startChan &&
                                chan < endChan &&
                                chunk->antenna1()(row) == static_cast<uint32_t>(ant1) &&
                                chunk->antenna2()(row) == static_cast<uint32_t>(ant2) &&
                                chunk->beam1()(row) == vis.beamid &&
                                chunk->beam2()(row) == vis.beamid &&
                                pol == 0) {
                            // If this is one of the visibilities that were added above
                            CPPUNIT_ASSERT_EQUAL(false, chunk->flag()(row, chan, pol));
                        } else {
                            CPPUNIT_ASSERT_EQUAL(true, chunk->flag()(row, chan, pol));
                        }
                    }
                }
            }

            // Check scan index
            CPPUNIT_ASSERT_EQUAL(0u, chunk->scan());

            // Check stokes
            CPPUNIT_ASSERT(chunk->stokes()(0) == casa::Stokes::XX);
            CPPUNIT_ASSERT(chunk->stokes()(1) == casa::Stokes::XY);
            CPPUNIT_ASSERT(chunk->stokes()(2) == casa::Stokes::YX);
            CPPUNIT_ASSERT(chunk->stokes()(3) == casa::Stokes::YY);

            // Check frequency vector
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_CHANNELS_PER_SLICE),
                    chunk->frequency().size());
        }

    private:

        boost::shared_ptr< MergedSource > itsInstance;
        MockMetadataSource::ShPtr itsMetadataSrc;
        MockVisSource::ShPtr itsVisSrc;

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
