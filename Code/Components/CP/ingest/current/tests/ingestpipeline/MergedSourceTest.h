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
#include "boost/shared_ptr.hpp"
#include "ingestpipeline/sourcetask/test/MockMetadataSource.h"
#include "ingestpipeline/sourcetask/test/MockVisSource.h"
#include "cpcommon/VisDatagram.h"
#include "cpcommon/TosMetadata.h"
#include "cpcommon/TosMetadataAntenna.h"
#include "cpcommon/VisChunk.h"
#include "measures/Measures.h"

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
            itsInstance.reset(new MergedSource(itsMetadataSrc, itsVisSrc, 1));
        }

        void tearDown() {
            itsInstance.reset();
            itsVisSrc.reset();
            itsMetadataSrc.reset();
        }

        // Test the MockMetadataSource before using it
        void testMockMetadataSource() {
            const long time = 1234;
            boost::shared_ptr<TosMetadata> md(new TosMetadata(1, 1, 1));
            md->time(time);
            itsMetadataSrc->add(md);
            CPPUNIT_ASSERT(itsMetadataSrc->next() == md);
        };

        // Test the MockVisSource before using it
        void testMockVisSource() {
            const long time = 1234;
            boost::shared_ptr<VisDatagram> vis(new VisDatagram);
            vis->timestamp = time;
            itsVisSrc->add(vis);
            CPPUNIT_ASSERT(itsVisSrc->next() == vis);
        };

        void testSingle() {
            const unsigned long starttime = 1000000; // One second after epoch
            const unsigned long period = 5 * 1000 * 1000;
            const unsigned int nAntenna = 2;
            const unsigned  int nCoarseChan = 304;
            const unsigned int nBeam = 1;
            const unsigned int nCorr = N_POL;

            // Create a mock metadata object and program it, then
            // add to the MockMetadataSource
            TosMetadata metadata(nCoarseChan, nBeam, nCorr);
            metadata.time(starttime);
            metadata.period(period);

            // antenna_names
            for (unsigned int i = 0; i < nAntenna; ++i) {
                std::stringstream ss;
                ss << "ASKAP" << i;
                unsigned int id = metadata.addAntenna(ss.str());
                TosMetadataAntenna& ant = metadata.antenna(id);
                ant.onSource(true);
                ant.hwError(false);
            }

            // Make a copy of the metadata and add it to the mock
            // Metadata source
            boost::shared_ptr<TosMetadata> copy(new TosMetadata(metadata));
            itsMetadataSrc->add(copy);

            // Populate a VisDatagram to match the metadata
            askap::cp::VisDatagram vis;
            vis.version = VISPAYLOAD_VERSION;
            vis.coarseChannel = 1;
            vis.antenna1 = 0;
            vis.antenna2 = 1;
            vis.beam1 = 0;
            vis.beam2 = 0;
            vis.timestamp = starttime;

            for (unsigned int i = 0; i < N_FINE_PER_COARSE * N_POL; ++i) {
                vis.nSamples[i] = 1;
            }

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
            const double midpoint = 3.5;
            casa::Quantity chunkMidpoint = chunk->time().getTime();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(midpoint, chunkMidpoint.getValue("s"), 1.0E-10);

            // Ensure other metadata is as expected
            CPPUNIT_ASSERT_EQUAL(nCoarseChan * N_FINE_PER_COARSE, chunk->nChannel());
            CPPUNIT_ASSERT_EQUAL(nCorr, chunk->nPol());
            const casa::uInt nBaselines = nAntenna * (nAntenna + 1) / 2;
            CPPUNIT_ASSERT_EQUAL(nBaselines * nBeam, chunk->nRow());

            // Ensure the visibilities that were supplied (most were not)
            // are not flagged, and that the rest are flagged

            // First calculate the channel range that was set
            const unsigned int startChan = vis.coarseChannel * N_FINE_PER_COARSE; //inclusive
            const unsigned int endChan = (vis.coarseChannel + 1) * N_FINE_PER_COARSE; //exclusive

            for (unsigned int row = 0; row < chunk->nRow(); ++row) {
                for (unsigned int chan = 0; chan < chunk->nChannel(); ++chan) {
                    for (unsigned int pol = 0; pol < chunk->nPol(); ++pol) {
                        if (chan >= startChan &&
                                chan < endChan &&
                                chunk->antenna1()(row) == vis.antenna1 &&
                                chunk->antenna2()(row) == vis.antenna2 &&
                                chunk->beam1()(row) == vis.beam1 &&
                                chunk->beam2()(row) == vis.beam2) {
                            // If this is one of the visibilities that were added above
                            CPPUNIT_ASSERT_EQUAL(false, chunk->flag()(row, chan, pol));
                        } else {
                            CPPUNIT_ASSERT_EQUAL(true, chunk->flag()(row, chan, pol));
                        }
                    }
                }
            }

            // Check stokes
            CPPUNIT_ASSERT(chunk->stokes()(0) == casa::Stokes::XX);
            CPPUNIT_ASSERT(chunk->stokes()(1) == casa::Stokes::XY);
            CPPUNIT_ASSERT(chunk->stokes()(2) == casa::Stokes::YX);
            CPPUNIT_ASSERT(chunk->stokes()(3) == casa::Stokes::YY);
        }

    private:

        boost::shared_ptr< MergedSource > itsInstance;
        MockMetadataSource::ShPtr itsMetadataSrc;
        MockVisSource::ShPtr itsVisSrc;

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
