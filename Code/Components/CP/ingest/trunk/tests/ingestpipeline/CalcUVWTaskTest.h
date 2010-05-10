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
#include "Common/ParameterSet.h"
#include "ingestpipeline/datadef/VisChunk.h"
#include "measures/Measures.h"
#include "measures/Measures/MEpoch.h"
#include "measures/Measures/MDirection.h"
#include "casa/Quanta/MVEpoch.h"
#include "casa/Arrays/Vector.h"

// Classes to test
#include "ingestpipeline/calcuvwtask/CalcUVWTask.h"

using namespace casa;

namespace askap {
namespace cp {

class CalcUVWTaskTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CalcUVWTaskTest);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp()
    {
        // Setup a parameter set
        itsParset.add("uvw.antennas.location", "[+117.471deg, -25.692deg, 192m, WGS84]");
        itsParset.add("uvw.antennas.names", "[A0, A1, A2, A3, A4, A5]");
        itsParset.add("uvw.antenna.scale", "1.0");
        itsParset.add("uvw.antennas.A0", "[-175.233429,  -1673.460938,  0.0000]");
        itsParset.add("uvw.antennas.A1", "[261.119019,   -796.922119,   0.0000]");
        itsParset.add("uvw.antennas.A2", "[-29.200520,   -744.432068,   0.0000]");
        itsParset.add("uvw.antennas.A3", "[-289.355286,  -586.936035,   0.0000]");
        itsParset.add("uvw.antennas.A4", "[-157.031570,  -815.570068,   0.0000]");
        itsParset.add("uvw.antennas.A5", "[-521.311646,  -754.674927,   0.0000]");
    };

    void tearDown()
    {
        itsParset.clear();
    }

    void testSimple()
    {
        const unsigned int row = 0;
        MEpoch starttime(MVEpoch(Quantity(50237.29, "d")),
                MEpoch::Ref(MEpoch::UTC));
        MDirection fieldCenter(Quantity( 20, "deg"),
                               Quantity(-10, "deg"),
                               MDirection::Ref(MDirection::J2000));

        // Create a simple chunk with 1 row, 1 channel and 1 pol
        VisChunk::ShPtr chunk(new VisChunk(1, 1, 1));
        chunk->time() = starttime.getValue();
        chunk->antenna1()(row) = 0;
        chunk->antenna2()(row) = 1;
        chunk->feed1()(row) = 0;
        chunk->feed2()(row) = 0;
        chunk->feed1PA()(row) = 0.0;
        chunk->feed2PA()(row) = 0.0;
        chunk->pointingDir1()(row) = fieldCenter;
        chunk->pointingDir2()(row) = fieldCenter;
        chunk->dishPointing1()(row) = fieldCenter;
        chunk->dishPointing2()(row) = fieldCenter;
        chunk->frequency()(0) = 1400000;

        // Instantiate the class under test and call process() to
        // add UVW coordinates to the VisChunk
        CalcUVWTask task(itsParset);
        task.process(chunk);

        CPPUNIT_ASSERT_EQUAL(1u, chunk->nRow());
        CPPUNIT_ASSERT(chunk->uvw().size() == 1);
        casa::RigidVector<casa::Double, 3> uvw = chunk->uvw()(row);

        // Tolerance for uvw equality
        const double tol = 1.0E-8;
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-347.517826227471, uvw(0), tol); //u
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-698.816518342588, uvw(1), tol); //v
        CPPUNIT_ASSERT_DOUBLES_EQUAL(591.278777468775, uvw(2), tol); //w
    };

    private:

    LOFAR::ParameterSet itsParset;

};

}   // End namespace cp

}   // End namespace askap
