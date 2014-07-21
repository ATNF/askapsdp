/// @file NoMetadataSourceTest.h
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

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include <cmath>
#include "boost/shared_ptr.hpp"
#include "Common/ParameterSet.h"
#include "ingestpipeline/sourcetask/test/MockVisSource.h"
#include "cpcommon/VisDatagram.h"
#include "cpcommon/VisChunk.h"
#include "ConfigurationHelper.h"

// Classes to test
#include "ingestpipeline/sourcetask/NoMetadataSource.h"

using askap::cp::common::VisChunk;

namespace askap {
namespace cp {
namespace ingest {

class NoMetadataSourceTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(NoMetadataSourceTest);
        //CPPUNIT_TEST(testMockVisSource);
        //CPPUNIT_TEST(testSingle);
        CPPUNIT_TEST_SUITE_END();

    public:

        void setUp() {
            itsVisSrc.reset(new MockVisSource);

            LOFAR::ParameterSet params;
            std::ostringstream ss;
            ss << N_CHANNELS_PER_SLICE;
            params.add("n_channels.0", ss.str());
            Configuration config = ConfigurationHelper::createDummyConfig();
            itsInstance.reset(new NoMetadataSource(params, config, itsVisSrc, 1, 0));
        }

        void tearDown() {
            itsInstance.reset();
            itsVisSrc.reset();
        }

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
            const unsigned int nBeam = 1;
            const unsigned int nCorr = 4;

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
            const double midpoint = 3.5;
            casa::Quantity chunkMidpoint = chunk->time().getTime();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(midpoint, chunkMidpoint.getValue("s"), 1.0E-10);

            // Ensure other metadata is as expected
            CPPUNIT_ASSERT_EQUAL(1 * N_CHANNELS_PER_SLICE, chunk->nChannel());
            CPPUNIT_ASSERT_EQUAL(nCorr, chunk->nPol());
            const casa::uInt nBaselines = nAntenna * (nAntenna + 1) / 2;
            CPPUNIT_ASSERT_EQUAL(nBaselines * nBeam, chunk->nRow());

            // Ensure the visibilities that were supplied (most were not)
            // are not flagged, and that the rest are flagged

            // First calculate the channel range that was set
            const unsigned int startChan = vis.slice * N_CHANNELS_PER_SLICE; //inclusive
            const unsigned int endChan = (vis.slice + 1) * N_CHANNELS_PER_SLICE; //exclusive

            for (unsigned int row = 0; row < chunk->nRow(); ++row) {
                for (unsigned int chan = 0; chan < chunk->nChannel(); ++chan) {
                    for (unsigned int pol = 0; pol < chunk->nPol(); ++pol) {
                        if (chan >= startChan &&
                                chan < endChan &&
                                chunk->antenna1()(row) == 0 &&
                                chunk->antenna2()(row) == 0 &&
                                chunk->beam1()(row) == vis.beamid &&
                                chunk->beam2()(row) == vis.beamid) {
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

        boost::shared_ptr< NoMetadataSource > itsInstance;
        MockVisSource::ShPtr itsVisSrc;

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
