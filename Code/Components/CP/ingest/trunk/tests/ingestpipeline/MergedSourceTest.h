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
#include "boost/shared_ptr.hpp"
#include "ingestpipeline/sourcetask/test/MockMetadataSource.h"
#include "ingestpipeline/sourcetask/test/MockVisSource.h"
#include "cpcommon/VisPayload.h"
#include "CommonTypes.h"
#include "TypedValues.h"

// Classes to test
#include "ingestpipeline/sourcetask/MergedSource.h"

// Using
using namespace askap::interfaces;

namespace askap
{
    namespace cp
    {
        class MergedSourceTest : public CppUnit::TestFixture
        {
            CPPUNIT_TEST_SUITE(MergedSourceTest);
            CPPUNIT_TEST(testMockMetadataSource);
            CPPUNIT_TEST(testMockVisSource);
            CPPUNIT_TEST(testSimple);
            CPPUNIT_TEST_SUITE_END();

            public:

            void setUp()
            {
                itsMetadataSrc.reset(new MockMetadataSource);
                itsVisSrc.reset(new MockVisSource);
                instance.reset(new MergedSource(itsMetadataSrc, itsVisSrc));
            }

            void tearDown()
            {
                instance.reset();
                itsVisSrc.reset();
                itsMetadataSrc.reset();
            }

            // Test the MockMetadataSource before using it
            void testMockMetadataSource()
            {
                const long time = 1234;
                boost::shared_ptr<TimeTaggedTypedValueMap> md(new TimeTaggedTypedValueMap);
                md->timestamp = time;
                itsMetadataSrc->add(md);
                CPPUNIT_ASSERT(itsMetadataSrc->next() == md);
            };

            // Test the MockVisSource before using it
            void testMockVisSource()
            {
                const long time = 1234;
                boost::shared_ptr<VisPayload> vis(new VisPayload);
                vis->timestamp = time;
                itsVisSrc->add(vis);
                CPPUNIT_ASSERT(itsVisSrc->next() == vis);
            };

            void testSimple()
            {
                const long timestamp = 1234;
                const long period = 5 * 1000 * 1000;
                const int nAntenna = 2;
                const unsigned  int nCoarseChan = 304;
                const int nBeam = 1;
                const int nCorr = N_POL;

                // Create a mock metadata object and program it, then
                // add to the MockMetadataSource
                boost::shared_ptr<TimeTaggedTypedValueMap> metadata(new TimeTaggedTypedValueMap);
                metadata->timestamp = timestamp;
                metadata->data["time"] = new TypedValueLong(TypeLong, timestamp);
                metadata->data["period"] = new TypedValueLong(TypeLong, period);
                metadata->data["n_coarse_chan"] = new TypedValueInt(TypeInt, nCoarseChan);
                metadata->data["n_antennas"] = new TypedValueInt(TypeInt, nAntenna);

                // n_beams
                {
                    IntSeq iseq;
                    iseq.assign(nCoarseChan, nBeam);
                    TypedValueIntSeqPtr tv = new TypedValueIntSeq(TypeIntSeq, iseq);
                    metadata->data["n_beams"] = tv;
                }

                // n_pol
                {
                    metadata->data["n_pol"] = new TypedValueInt(TypeInt, nCorr);
                }

                // antenna_names
                {
                    StringSeq sseq;
                    for (int i = 0; i < nAntenna; ++i) {
                        std::stringstream ss;
                        ss << "ASKAP" << i;
                        sseq.push_back(ss.str());
                    }

                    TypedValueStringSeqPtr tv = new TypedValueStringSeq(TypeStringSeq, sseq);
                    metadata->data["antenna_names"] = tv;
                }


                itsMetadataSrc->add(metadata);

                // Create a mock vis and program it, then add to the
                // MockVisSource

                // Populate the VisPayload
                askap::cp::VisPayload payload;
                payload.version = VISPAYLOAD_VERSION;
                payload.timestamp = timestamp;

                for (int ant1 = 0; ant1 < nAntenna; ant1++) {
                    for (int ant2 = 0; ant2 < nAntenna; ant2++) {
                        payload.antenna1 = ant1;
                        payload.antenna2 = ant2;
                        payload.beam1 = 1;
                        payload.beam2 = 1;

                        // Set all nSamples to 1 and ensure nominalNSamples is also 1
                        for (unsigned int i = 0; i < N_FINE_PER_COARSE * N_POL; ++i) {
                            payload.nSamples[i] = 1;
                        }

                        for (unsigned int coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
                            payload.coarseChannel = coarseChan + 1;
                            for (unsigned int fineChan = 0; fineChan < N_FINE_PER_COARSE; ++fineChan) {
                                for (unsigned int pol = 0; pol < N_POL; ++pol) {
                                    const int idx = pol + (N_POL * fineChan);
                                    payload.vis[idx].real = 1.0;
                                    payload.vis[idx].imag = 2.0;
                                }
                            }
                            // Finished populating, save this payload but then reuse it in the
                            // next iteration of the loop for the next coarse channel
                            boost::shared_ptr<VisPayload> vis(new VisPayload(payload));
                            itsVisSrc->add(vis);
                        }
                    }
                }

                // Now the data has been added to the two data sources, can use the
                // MergedSource
                VisChunk::ShPtr vischunk(instance->next());
                CPPUNIT_ASSERT(vischunk.get() != 0);
            }

        private:
        boost::shared_ptr< MergedSource > instance;
        MockMetadataSource::ShPtr itsMetadataSrc;
        MockVisSource::ShPtr itsVisSrc;

    };

}   // End namespace cp

}   // End namespace askap
