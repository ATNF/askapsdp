/// @file VisChunkTest.cc
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
#include "askap/AskapError.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Cube.h"

// Classes to test
#include "cpcommon/VisChunk.h"

using namespace casa;
using askap::cp::common::VisChunk;

namespace askap {
namespace cp {
namespace ingest {

class VisChunkTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(VisChunkTest);
        CPPUNIT_TEST(testConstructor);
        CPPUNIT_TEST(testResizeChans);
        CPPUNIT_TEST(testResizeRows);
        CPPUNIT_TEST(testResizePols);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testConstructor() {
            VisChunk::ShPtr chunk(new VisChunk(nRows, nChans, nPols));
            CPPUNIT_ASSERT_EQUAL(nRows, chunk->nRow());
            CPPUNIT_ASSERT_EQUAL(nChans, chunk->nChannel());
            CPPUNIT_ASSERT_EQUAL(nPols, chunk->nPol());

            // Verify visibility cube
            CPPUNIT_ASSERT_EQUAL(nRows, chunk->visibility().nrow());
            CPPUNIT_ASSERT_EQUAL(nChans, chunk->visibility().ncolumn());
            CPPUNIT_ASSERT_EQUAL(nPols, chunk->visibility().nplane());

            // Verify flag cube
            CPPUNIT_ASSERT_EQUAL(nRows, chunk->flag().nrow());
            CPPUNIT_ASSERT_EQUAL(nChans, chunk->flag().ncolumn());
            CPPUNIT_ASSERT_EQUAL(nPols, chunk->flag().nplane());

            // Verify frequency vector
            CPPUNIT_ASSERT_EQUAL(nChans,
                                 static_cast<unsigned int>(chunk->frequency().size()));
        }

        void testResizeChans() {
            resizeDriver(nRows, nChans, nPols,
                         nRows, 304, nPols);
        }

        void testResizeRows() {
            CPPUNIT_ASSERT_THROW(
                resizeDriver(nRows, nChans, nPols, nRows + 1, nChans, nPols),
                askap::AskapError);
        }

        void testResizePols() {
            CPPUNIT_ASSERT_THROW(
                resizeDriver(nRows, nChans, nPols, nRows, nChans, nPols + 1),
                askap::AskapError);

        }


    private:
        void resizeDriver(const unsigned int initialRows,
                          const unsigned int initialChans,
                          const unsigned int initialPols,
                          const unsigned int newRows,
                          const unsigned int newChans,
                          const unsigned int newPols) {
            VisChunk::ShPtr chunk(new VisChunk(initialRows, initialChans, initialPols));

            // Create and assign the containers
            casa::Cube<casa::Complex> vis(newRows, newChans, newPols);
            casa::Cube<casa::Bool> flag(newRows, newChans, newPols);
            casa::Vector<casa::Double> frequency(newChans);
            chunk->resize(vis, flag, frequency);

            // Verify the result
            CPPUNIT_ASSERT_EQUAL(newRows, chunk->nRow());
            CPPUNIT_ASSERT_EQUAL(newChans, chunk->nChannel());
            CPPUNIT_ASSERT_EQUAL(newPols, chunk->nPol());

            // Verify visibility cube
            CPPUNIT_ASSERT_EQUAL(newRows, chunk->visibility().nrow());
            CPPUNIT_ASSERT_EQUAL(newChans, chunk->visibility().ncolumn());
            CPPUNIT_ASSERT_EQUAL(newPols, chunk->visibility().nplane());

            // Verify flag cube
            CPPUNIT_ASSERT_EQUAL(newRows, chunk->flag().nrow());
            CPPUNIT_ASSERT_EQUAL(newChans, chunk->flag().ncolumn());
            CPPUNIT_ASSERT_EQUAL(newPols, chunk->flag().nplane());

            // Verify frequency vector
            CPPUNIT_ASSERT_EQUAL(newChans,
                                 static_cast<unsigned int>(chunk->frequency().size()));
        }

        //
        // Test values
        //

        // This is the size of a BETA VisChunk, 21 baselines (including
        // auto correlations) * 36 beams (maximum number of beams)
        static const unsigned int nRows = 21 * 36;

        // 304 coarse channels with 54 fine channels per coarse
        static const unsigned int nChans = 54 * 304;

        // Polarisations
        static const unsigned int nPols = 4;
};

const unsigned int VisChunkTest::nRows;
const unsigned int VisChunkTest::nChans;
const unsigned int VisChunkTest::nPols;

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
