/// @file CalcUVWTaskTest.cc
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
#include "ingestpipeline/calcuvwtask/CalcUVWTask.h"

using namespace casa;
using askap::cp::common::VisChunk;

namespace askap {
namespace cp {
namespace ingest {

class CalcUVWTaskTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(CalcUVWTaskTest);
        CPPUNIT_TEST(testOffset);
        CPPUNIT_TEST(testAutoCorrelation);
        CPPUNIT_TEST(testInvalidAntenna);
        CPPUNIT_TEST(testInvalidBeam);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testOffset() {
            //      ant1, ant2, beam,    u,      v,      w
            testDriver(0,    1,    0,    -411.4, -838.4, 294.1);
            testDriver(0,    2,    0,    -120.2, -874.0, 325.5);

            testDriver(0,    1,    1,    -411.9, -843.1, 279.8);
            testDriver(0,    2,    1,    -120.7, -879.4, 310.4);
        }

        void testAutoCorrelation() {
            //      ant1, ant2, beam,    u,   v,   w
            testDriver(0,    0,    0,    0.0, 0.0, 0.0);
        }

        void testInvalidAntenna() {
            CPPUNIT_ASSERT_THROW(
                //       ant1, ant2, beam,    u,   v,   w
                testDriver(7,    0,    0,    0.0, 0.0, 0.0),
                askap::AskapError);
        }

        void testInvalidBeam() {
            CPPUNIT_ASSERT_THROW(
                //      ant1, ant2, beam,    u,   v,   w
                testDriver(0,    0,    4,    0.0, 0.0, 0.0),
                askap::AskapError);
        }

        void testDriver(const unsigned int antenna1,
                        const unsigned int antenna2,
                        const unsigned int beam,
                        const double u,
                        const double v,
                        const double w) {
            const unsigned int row = 0;
            MEpoch starttime(MVEpoch(Quantity(54165.73871, "d")),
                             MEpoch::Ref(MEpoch::UTC));
            MDirection fieldCenter(Quantity(187.5, "deg"),
                                   Quantity(-45, "deg"),
                                   MDirection::Ref(MDirection::J2000));

            // Create a simple chunk with 1 row, 1 channel and 1 pol
            VisChunk::ShPtr chunk(new VisChunk(1, 1, 1));
            chunk->time() = starttime.getValue();
            chunk->antenna1()(row) = antenna1;
            chunk->antenna2()(row) = antenna2;
            chunk->beam1()(row) = beam;
            chunk->beam2()(row) = beam;
            chunk->beam1PA()(row) = 0.0;
            chunk->beam2PA()(row) = 0.0;
            chunk->pointingDir1()(row) = fieldCenter.getAngle();
            chunk->pointingDir2()(row) = fieldCenter.getAngle();
            chunk->dishPointing1()(row) = fieldCenter.getAngle();
            chunk->dishPointing2()(row) = fieldCenter.getAngle();
            chunk->frequency()(0) = 1400000;

            // Instantiate the class under test and call process() to
            // add UVW coordinates to the VisChunk
            CalcUVWTask task(itsParset, createTestConfig());
            task.process(chunk);

            CPPUNIT_ASSERT_EQUAL(1u, chunk->nRow());
            CPPUNIT_ASSERT(chunk->uvw().size() == 1);
            casa::RigidVector<casa::Double, 3> uvw = chunk->uvw()(row);

            // Tolerance for uvw equality
            const double tol = 1.0E-1;
            CPPUNIT_ASSERT_DOUBLES_EQUAL(u, uvw(0), tol); //u
            CPPUNIT_ASSERT_DOUBLES_EQUAL(v, uvw(1), tol); //v
            CPPUNIT_ASSERT_DOUBLES_EQUAL(w, uvw(2), tol); //w
        };

        Antenna createAntenna(const std::string& name, const casa::Double& x,
                const casa::Double& y, const casa::Double& z)
        {
            // Setup feed config
            const casa::uInt nFeeds = 4;
            const casa::uInt nReceptors = 2;
            const casa::Quantity spacing(1, "deg");
            casa::Matrix<casa::Quantity> offsets(nFeeds, nReceptors);
            casa::Vector<casa::String> pols(nFeeds, "X Y");
            offsets(0, 0) = spacing * -2.5;
            offsets(0, 1) = spacing * -1.5;

            offsets(1, 0) = spacing * -2.5;
            offsets(1, 1) = spacing * -0.5;

            offsets(2, 0) = spacing * -2.5;
            offsets(2, 1) = spacing * 0.5;

            offsets(3, 0) = spacing * -2.5;
            offsets(3, 1) = spacing * 1.5;

            FeedConfig paf4(offsets, pols);
           
            // Setup antenna
            const std::string mount = "equatorial";
            const casa::Quantity diameter(12.0, "m");

            casa::Vector<casa::Double> pos(3);
            pos(0) = x;
            pos(1) = y;
            pos(2) = z;
            return Antenna(name,
                    mount,
                    pos,
                    diameter,
                    paf4);
        };

        Configuration createTestConfig(void)
        {
            Configuration empty = ConfigurationHelper::createDummyConfig();

            // Setup antennas
            std::vector<Antenna> antennas;
            antennas.push_back(createAntenna("A0", -2652616.854602326, 5102312.637997697, -2749946.411592145));
            antennas.push_back(createAntenna("A1", -2653178.349042055, 5102446.673161191, -2749155.53718417));
            antennas.push_back(createAntenna("A2", -2652931.204894244, 5102600.67778301, -2749108.177002157));
            antennas.push_back(createAntenna("A3", -2652731.709913884, 5102780.937978324, -2748966.073105379));
            antennas.push_back(createAntenna("A4", -2652803.638192114, 5102632.431992128, -2749172.362663322));
            antennas.push_back(createAntenna("A5", -2652492.544738157, 5102823.769989723, -2749117.418823366));

            return Configuration(empty.arrayName(),
                    empty.tasks(),
                    antennas,
                    empty.correlatorModes(),
                    empty.observation(),
                    empty.metadataTopic(),
                    empty.calibrationDataService());
        };

    private:

        LOFAR::ParameterSet itsParset;
};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
