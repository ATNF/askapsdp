/// @file CalTaskTest.cc
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
/// @author Maxim Voronkov <maxim.voronkov@csiro.au>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include "ingestpipeline/datadef/VisChunk.h"
#include "Common/ParameterSet.h"
#include "measures/Measures.h"
#include "casa/Quanta/MVEpoch.h"

// Classes to test
#include "ingestpipeline/caltask/CalTask.h"

using namespace casa;

namespace askap {
namespace cp {

class CalTaskTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CalTaskTest);
    CPPUNIT_TEST(testCal);
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp()
    {
	itsParset.add("gain.g11.0", "[1.0]");
	itsParset.add("gain.g11.1", "[0.9,0.1]");
	itsParset.add("gain.g22.0", "[0.0,-0.5]");
	itsParset.add("gain.g22.1", "[0.9,0.1]");
    };

    void tearDown()
    {
        itsParset.clear();
    }

    void testCal()
    {
        const int row = 0;
        const int nRows = 1;
        const int nChans = 1;
        const int nPols = 4;
        MVEpoch time(Quantity(50237.29, "d"));

        VisChunk::ShPtr chunk(new VisChunk(nRows, nChans, nPols));
        chunk->time() = time;
        chunk->antenna1()(row) = 0;
        chunk->antenna2()(row) = 1;
        chunk->beam1()(row) = 0;
        chunk->beam2()(row) = 0;
	chunk->visibility().set(1.);

        CalTask task(itsParset);
        task.process(chunk);
        CPPUNIT_ASSERT_EQUAL(time, chunk->time());

    };

    private:

    LOFAR::ParameterSet itsParset;

};

}   // End namespace cp

}   // End namespace askap
