/// @file SubsetExtractorTest.cc
///
/// @copyright (c) 2014 CSIRO
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
#include <stdint.h>
#include <limits>
#include <complex>
#include <algorithm>
#include <vector>
#include <cstring>
#include "casa/Arrays/Cube.h"
#include "askap/AskapError.h"
#include "publisher/InputMessage.h"
#include "publisher/SpdOutputMessage.h"
#include "TestHelperFunctions.h"

// Classes to test
#include "publisher/SubsetExtractor.h"

using namespace std;

namespace askap {
namespace cp {
namespace vispublisher {

class SubsetExtractorTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(SubsetExtractorTest);
        CPPUNIT_TEST(testInIndex);
        CPPUNIT_TEST(testIndexOfFirst);
        CPPUNIT_TEST(testMakeAntennaVectors);
        CPPUNIT_TEST(testSubset);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            itsInMsg = TestHelperFunctions::createInputMessage();
            N_BEAM = TestHelperFunctions::N_BEAM;
            N_CHAN = TestHelperFunctions::N_CHAN;
            N_POL = TestHelperFunctions::N_POL;
            N_BASELINE = TestHelperFunctions::N_BASELINE;
        }

        void tearDown() {
        }

        void testSubset() {
            const float EPSILON = std::numeric_limits<float>::epsilon();

            for (uint32_t beam = 0; beam < N_BEAM; ++beam) {
                for (uint32_t pol = 0; pol < N_POL; ++pol) {
                    SpdOutputMessage out = SubsetExtractor::subset(itsInMsg, beam, pol);
                    CPPUNIT_ASSERT_EQUAL(itsInMsg.timestamp(), out.timestamp());
                    CPPUNIT_ASSERT_EQUAL(beam, out.beamId());
                    CPPUNIT_ASSERT_EQUAL(pol, out.polId());
                    CPPUNIT_ASSERT_EQUAL(itsInMsg.nChannels(), out.nChannels());

                    CPPUNIT_ASSERT_DOUBLES_EQUAL(itsInMsg.chanWidth(), out.chanWidth(), EPSILON);

                    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_CHAN), out.frequency().size());
                    CPPUNIT_ASSERT_EQUAL(N_BASELINE, out.nBaselines());

                    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_BASELINE), out.antenna1().size());
                    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_BASELINE), out.antenna2().size());

                    const std::vector< std::complex<float> >& vis = out.visibilities();
                    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_BASELINE * N_CHAN), vis.size());

                    const std::vector< uint8_t >& flag = out.flag();
                    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_BASELINE * N_CHAN), flag.size());

                    for (size_t baseline = 0; baseline < N_BASELINE; ++baseline) {
                        for (size_t chan = 0; chan < N_CHAN; ++chan) {
                            const size_t idx = chan + (N_CHAN * baseline);
                            CPPUNIT_ASSERT(idx < N_BASELINE * N_CHAN);
                            complex<float> expected = TestHelperFunctions::visgen(
                                    chan, out.antenna1()[baseline],
                                    out.antenna2()[baseline], beam, pol);
                            CPPUNIT_ASSERT_EQUAL(expected, vis[idx]);
                            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(1), flag[idx]);
                        }
                    }
                }
            }
        }

        void testIndexOfFirst() {
            std::vector<uint32_t> v;
            v.push_back(10);
            v.push_back(11);
            v.push_back(12);
            v.push_back(13);
            CPPUNIT_ASSERT_EQUAL(0ul, SubsetExtractor::indexOfFirst(v, 10));
            CPPUNIT_ASSERT_EQUAL(1ul, SubsetExtractor::indexOfFirst(v, 11));
            CPPUNIT_ASSERT_EQUAL(2ul, SubsetExtractor::indexOfFirst(v, 12));
            CPPUNIT_ASSERT_EQUAL(3ul, SubsetExtractor::indexOfFirst(v, 13));
        }

        void testInIndex() {
            const size_t N_ROW = N_BASELINE * N_BEAM;
            const size_t sz = N_ROW * N_CHAN * N_POL;

            // Ensure the entire range is within bounds
            for (size_t row = 0; row < N_ROW; ++row) {
                for (size_t chan = 0; chan < N_CHAN; ++chan) {
                    for (size_t pol = 0; pol < N_POL; ++pol) {
                        size_t idx = itsInMsg.index(row, chan, pol);
                        CPPUNIT_ASSERT(idx < sz);
                    }
                }
            }

            // Test first element
            CPPUNIT_ASSERT_EQUAL(0ul, itsInMsg.index(0, 0, 0));

            // Test last element
            CPPUNIT_ASSERT_EQUAL(sz - 1, itsInMsg.index(N_ROW - 1, N_CHAN - 1, N_POL - 1));
        }

        void testMakeAntennaVectors() {
            std::vector<uint32_t> ant1;
            std::vector<uint32_t> ant2;
            uint32_t nBaselines = SubsetExtractor::makeAntennaVectors(itsInMsg, 0, ant1, ant2);
            CPPUNIT_ASSERT_EQUAL(N_BASELINE, nBaselines);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_BASELINE), ant1.size());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_BASELINE), ant2.size());

            CPPUNIT_ASSERT_EQUAL(0u, ant1[0]);
            CPPUNIT_ASSERT_EQUAL(1u, ant2[0]);

            CPPUNIT_ASSERT_EQUAL(0u, ant1[1]);
            CPPUNIT_ASSERT_EQUAL(2u, ant2[1]);

            CPPUNIT_ASSERT_EQUAL(1u, ant1[2]);
            CPPUNIT_ASSERT_EQUAL(2u, ant2[2]);

            // Test a beam that does not appear in the data structure
            nBaselines = SubsetExtractor::makeAntennaVectors(itsInMsg, 99, ant1, ant2);
            CPPUNIT_ASSERT_EQUAL(0u, nBaselines);
            CPPUNIT_ASSERT_EQUAL(0ul, ant1.size());
            CPPUNIT_ASSERT_EQUAL(0ul, ant2.size());
        }

    private:

        InputMessage itsInMsg;
        uint32_t N_BEAM;
        uint32_t N_CHAN;
        uint32_t N_POL;
        uint32_t N_BASELINE;
};

}   // End namespace vispublisher
}   // End namespace cp
}   // End namespace askap
