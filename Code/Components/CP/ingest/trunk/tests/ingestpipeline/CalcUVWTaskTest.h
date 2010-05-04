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
#include "boost/shared_ptr.hpp"

// Classes to test
#include "ingestpipeline/calcuvwtask/CalcUVWTask.h"
#include "ingestpipeline/datadef/VisChunk.h"

namespace askap
{
    namespace cp
    {
        class CalcUVWTaskTest : public CppUnit::TestFixture
        {
            CPPUNIT_TEST_SUITE(CalcUVWTaskTest);
            CPPUNIT_TEST(testSimple);
            CPPUNIT_TEST_SUITE_END();

            public:
            void testSimple()
            {
                /*
                const unsigned long starttime = 1000000; // One second after epoch
                const unsigned long period = 5 * 1000 * 1000;
                const unsigned int nAntenna = 2;
                const unsigned  int nCoarseChan = 304;
                const unsigned int nBeam = 1;
                const unsigned int nCorr = N_POL;

                VisChunk::ShPtr chunk(new VisChunk);
                chunk->time() = starttime;
                */
            };

        };

    }   // End namespace cp

}   // End namespace askap
