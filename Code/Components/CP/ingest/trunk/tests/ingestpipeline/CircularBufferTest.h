/// @file CircularBufferTest.cc
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
#include "ingestpipeline/sourcetask/CircularBuffer.h"

namespace askap
{
    namespace cp
    {
        class CircularBufferTest : public CppUnit::TestFixture
        {
            CPPUNIT_TEST_SUITE(CircularBufferTest);
            CPPUNIT_TEST(testSingle);
            CPPUNIT_TEST(testMultiple);
            CPPUNIT_TEST(testOverflow);
            CPPUNIT_TEST(testTimeout);
            CPPUNIT_TEST_SUITE_END();

            public:
            // Test the addition and retrieval of a single pointer
            void testSingle()
            {
                CircularBuffer<int> instance(2);
                boost::shared_ptr<int> inPtr(new int);
                *inPtr = 1;
                instance.add(inPtr);

                boost::shared_ptr<int> outPtr(instance.next());
                CPPUNIT_ASSERT_EQUAL(*inPtr, *outPtr);
            };

            // Test the addition and retrieval of a large number of pointers
            void testMultiple()
            {
                const int count = 1024 * 1024;
                CircularBuffer<int> instance(count);
                for (int i = 0; i < count; ++i) {
                    boost::shared_ptr<int> inPtr(new int);
                    *inPtr = count;
                    instance.add(inPtr);
                }

                for (int i = 0; i < count; ++i) {
                    boost::shared_ptr<int> outPtr(instance.next());
                    CPPUNIT_ASSERT_EQUAL(count, *outPtr);
                }
            };

            // Test the addition of more pointers than the buffer has capacity
            // to handle
            void testOverflow()
            {
                const int count = 1024;
                CircularBuffer<int> instance(10);
                for (int i = 0; i < count; ++i) {
                    boost::shared_ptr<int> inPtr(new int);
                    *inPtr = count;
                    instance.add(inPtr);
                }
            };

            // Test the timeout parameter. Just make sure this does not block
            // forever.
            void testTimeout()
            {
                const long timeout = 10; // milliseconds
                CircularBuffer<int> instance(2);

                boost::shared_ptr<int> outPtr(instance.next(timeout));
                CPPUNIT_ASSERT(outPtr.get() == 0);
            };


        };

    }   // End namespace cp

}   // End namespace askap
