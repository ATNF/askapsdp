/// @file ChannelAvgTaskTest.cc
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
#include <sstream>
#include <cmath>
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"
#include "measures/Measures.h"
#include "measures/Measures/MEpoch.h"
#include "measures/Measures/MDirection.h"
#include "casa/Quanta/MVEpoch.h"
#include "casa/Arrays/Vector.h"
#include "configuration/Configuration.h"
#include "ConfigurationHelper.h"

// Classes to test
#include "ingestpipeline/chanavgtask/ChannelAvgTask.h"

using namespace casa;
using askap::cp::common::VisChunk;

namespace askap {
namespace cp {
namespace ingest {

class ChannelAvgTaskTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(ChannelAvgTaskTest);
        CPPUNIT_TEST(testFourToOne);
        CPPUNIT_TEST(testFiftyFourToOne);
        CPPUNIT_TEST(testEightToTwo);
        CPPUNIT_TEST(testFullFineToCoarse);
        CPPUNIT_TEST(testNoAveraging);
        CPPUNIT_TEST(testInvalid);
        CPPUNIT_TEST(testAllFlagged);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
            itsParset.clear();
        }

        void testFourToOne() {
            averageTest(4, 4);
        }

        void testFiftyFourToOne() {
            averageTest(54, 54);
        }

        void testEightToTwo() {
            averageTest(8, 4);
        }

        void testFullFineToCoarse() {
            averageTest(304 * 54, 304);
        }

        // Test where no averaging is requested. The output VisChunk should be
        // identical to the input.
        void testNoAveraging() {
            averageTest(304 * 54, 1);
        }

        // Test where all visibilities are flagged. This ensures no divide by
        // zero occurs
        void testAllFlagged() {
            averageTest(304 * 54, 304, true);
        }

        void testInvalid() {
            // This is an invalid configuraion, so should throw an exception
            CPPUNIT_ASSERT_THROW(averageTest(4, 3), askap::AskapError);
        }

        /// Generic avergaing test driver
        /// @param[in] nChan            number of spectral channels to create
        /// @param[in] channelAveraging number of channels to average
        ///                             together to form one. This must evenly
        ///                             divide nChan
        /// @param[in] allFlagged       if true, the test drvier creates a VisChunk
        ///                             with all visibilities flagged. This is an edge
        ///                             case that can often result in a divide-by-zero.
        void averageTest(const unsigned int nChan, const unsigned int channelAveraging,
                         bool allFlagged = false) {
            // Setup the parset for the channel averaging task
            std::ostringstream ss;
            ss << channelAveraging;
            itsParset.add("averaging", ss.str());

            const unsigned int row = 0;
            const double startFreq = 1.4 * 1000 * 1000; // Hz
            const double freqInc = 18.5 * 1000; // Hz
            MEpoch starttime(MVEpoch(Quantity(50237.29, "d")),
                             MEpoch::Ref(MEpoch::UTC));
            MDirection fieldCenter(Quantity(20, "deg"),
                                   Quantity(-10, "deg"),
                                   MDirection::Ref(MDirection::J2000));

            // Create a simple chunk with 1 row,  nChan channels and 1 pol
            VisChunk::ShPtr chunk(new VisChunk(1, nChan, 1));
            chunk->time() = starttime.getValue();
            chunk->antenna1()(row) = 0;
            chunk->antenna2()(row) = 1;
            chunk->beam1()(row) = 0;
            chunk->beam2()(row) = 0;
            chunk->beam1PA()(row) = 0.0;
            chunk->beam2PA()(row) = 0.0;
            chunk->pointingDir1()(row) = fieldCenter.getAngle();
            chunk->pointingDir2()(row) = fieldCenter.getAngle();
            chunk->dishPointing1()(row) = fieldCenter.getAngle();
            chunk->dishPointing2()(row) = fieldCenter.getAngle();
            chunk->channelWidth() = freqInc;

            // Determine how many channels will exist after averaging
            casa::uInt nChanNew = nChan / channelAveraging;

            // To support the "invalid configuration test"
            if (nChan % channelAveraging != 0) {
                nChanNew += channelAveraging;
            }

            // Add the VisChunk is built (below) keep track of the sums
            // in these vectors so they can later be used to determine
            // the averages.
            casa::Vector<casa::Complex> visSum(nChanNew, 0.0);
            casa::Vector<double> freqSum(nChanNew, 0.0);

            // Add visibilities and unset flag
            // also keep track of the sums of visibilities and frequencies
            // for each of the new channels
            const unsigned int pol = 0;
            unsigned int newIdx = 0;

            for (unsigned int chan = 0; chan < nChan; ++chan) {
                if ((chan != 0) && (chan % channelAveraging == 0)) {
                    newIdx++;
                }

                casa::Complex val(static_cast<float>(chan + 1),
                                  static_cast<float>(chan + 2));
                chunk->visibility()(row, chan, pol) = val;
                visSum(newIdx) += val;
                chunk->flag()(row, chan, pol) = allFlagged;

                // Also set frequency information
                chunk->frequency()(chan) = startFreq + (chan * freqInc);
                freqSum(newIdx) += chunk->frequency()(chan);
            }

            // Check pre-conditions
            CPPUNIT_ASSERT_EQUAL(nChan, chunk->nChannel());

            // Instantiate the class under test and call process() to
            // average channels in the VisChunk
            ChannelAvgTask task(itsParset, ConfigurationHelper::createDummyConfig());
            task.process(chunk);

            // Tolerance for double equality
            const double tol = 1.0E-10;

            // Iterate over each of the new channels
            for (unsigned int i = 0; i < nChanNew; ++i) {
                // Determine the values for post-conditions
                float expectedReal = visSum(i).real() / channelAveraging;
                float expectedImag = visSum(i).imag() / channelAveraging;
                double expectedFreq = freqSum(i) / channelAveraging;
                bool expectedFlag = allFlagged;

                // Check post-conditions
                CPPUNIT_ASSERT_EQUAL(1u, chunk->nRow());
                CPPUNIT_ASSERT_EQUAL(nChanNew, chunk->nChannel());
                CPPUNIT_ASSERT_EQUAL(nChanNew,
                                     static_cast<unsigned int>(chunk->frequency().size()));
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expectedFreq, chunk->frequency()(i), tol);

                CPPUNIT_ASSERT(!std::isnan(chunk->visibility()(row, i, pol).real()));
                CPPUNIT_ASSERT(!std::isnan(chunk->visibility()(row, i, pol).imag()));

                if (allFlagged) {
                    expectedReal = 0.0;
                    expectedImag = 0.0;
                }
                CPPUNIT_ASSERT_EQUAL(expectedFlag, chunk->flag()(row, i, pol));
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expectedReal,
                        chunk->visibility()(row, i, pol).real(), tol);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expectedImag,
                        chunk->visibility()(row, i, pol).imag(), tol);
            }

            // Check the channel width has been updated
            CPPUNIT_ASSERT_DOUBLES_EQUAL(freqInc * channelAveraging, chunk->channelWidth(), tol);
        };

    private:

        LOFAR::ParameterSet itsParset;
};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
