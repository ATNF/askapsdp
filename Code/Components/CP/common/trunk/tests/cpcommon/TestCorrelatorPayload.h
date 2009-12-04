/// @file TestCorrelatorPayload.h
///
/// @copyright (c) 2009 CSIRO
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

// Classes to test
#include <cpcommon/CorrelatorPayload.h>

// System includes
#include <vector>
#include <string>
#include <limits>

// ASKAPsoft includes
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufVector.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobOBufVector.h>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <measures/Measures/Stokes.h>

namespace askap
{
    namespace cp
    {
        class TestCorrelatorPayload : public CppUnit::TestFixture
        {
            CPPUNIT_TEST_SUITE(TestCorrelatorPayload);
            CPPUNIT_TEST(testEmpty);
            CPPUNIT_TEST(testLowerBounds);
            CPPUNIT_TEST(testUpperBounds);
            CPPUNIT_TEST(testNominal);
            CPPUNIT_TEST_SUITE_END();

            public:

            // Compare two casa::Arrays for conformance and identical
            // contents
            template<typename T>
            bool equalArray(const casa::Array<T>& a, const casa::Array<T>& b)
            {
                if (!a.conform(b)) {
                    return false;
                }

                typename casa::Array<T>::const_iterator aIter(a.begin());
                typename casa::Array<T>::const_iterator bIter(b.begin());

                while (aIter != a.end()) {
                    if (*aIter != *bIter) {
                        return false;
                    }
                    ++aIter;
                    ++bIter;
                }

                return true;
            }

            // Serialize a CorrelatorPayload, deserialize it then
            // compare the two are identical
            void sendAndCompare(const CorrelatorPayload& payload)
            {
                const int c_version = 1;
                const std::string c_name = "TestMessage";

                // Encode
                std::vector<int8_t> buf;
                {
                    LOFAR::BlobOBufVector<int8_t> bv(buf);
                    LOFAR::BlobOStream out(bv);
                    out.putStart(c_name, c_version);
                    out << payload;
                    out.putEnd();
                }

                // Decode
                CorrelatorPayload outputPayload;
                {
                    LOFAR::BlobIBufVector<int8_t> bv(buf);
                    LOFAR::BlobIStream in(bv);
                    int version = in.getStart(c_name);
                    CPPUNIT_ASSERT(version == c_version);
                    in >> outputPayload;
                    in.getEnd();
                }

                CPPUNIT_ASSERT(payload.timestamp == outputPayload.timestamp);
                CPPUNIT_ASSERT(payload.coarseChannel == outputPayload.coarseChannel);
                CPPUNIT_ASSERT(payload.nRow == outputPayload.nRow);
                CPPUNIT_ASSERT(payload.nChannel == outputPayload.nChannel);
                CPPUNIT_ASSERT(payload.nPol == outputPayload.nPol);

                CPPUNIT_ASSERT(equalArray(payload.antenna1, outputPayload.antenna1));
                CPPUNIT_ASSERT(equalArray(payload.antenna2, outputPayload.antenna2));
                CPPUNIT_ASSERT(equalArray(payload.beam1, outputPayload.beam1));
                CPPUNIT_ASSERT(equalArray(payload.beam2, outputPayload.beam2));
                CPPUNIT_ASSERT(equalArray(payload.polarisations, outputPayload.polarisations));
                CPPUNIT_ASSERT(equalArray(payload.vis, outputPayload.vis));
                CPPUNIT_ASSERT(equalArray(payload.nSamples, outputPayload.nSamples));

                CPPUNIT_ASSERT(payload.nominalNSamples == outputPayload.nominalNSamples);
            };

            // Test an uninitialized payload
            void testEmpty()
            {
                CorrelatorPayload payload;
                sendAndCompare(payload);
            };

            // Test for boundary conditions on lower bounds
            void testLowerBounds()
            {
                CorrelatorPayload payload;

                payload.timestamp = 0;
                payload.coarseChannel = 0;
                payload.nRow = 0;
                payload.nChannel = 0;
                payload.nPol = 0;
                payload.nominalNSamples = 0;

                sendAndCompare(payload);
            };

            // Test for boundary conditions on upper bounds
            void testUpperBounds()
            {
                const unsigned int c_maxulong = std::numeric_limits<unsigned long>::max();
                const unsigned int c_maxuint = std::numeric_limits<unsigned int>::max();
                CorrelatorPayload payload;

                payload.timestamp = c_maxulong;
                payload.coarseChannel = c_maxuint;
                payload.nRow = c_maxuint;
                payload.nChannel = c_maxuint;
                payload.nPol = c_maxuint;
                payload.nominalNSamples = c_maxuint;

                sendAndCompare(payload);
            };

            // Test a nominal type payload
            void testNominal()
            {
                const int nRow = (36*37)/2 * 32;
                const int nPol = 4;
                const int nChannel = 54;

                CorrelatorPayload payload;
                payload.timestamp = 123456789;
                payload.coarseChannel = 101;
                payload.nRow = nRow;
                payload.nChannel = nChannel;
                payload.nPol = nPol;

                payload.antenna1.resize(nRow);
                payload.antenna1(1) = 1234;
                payload.antenna2.resize(nRow);
                payload.antenna1(1) = 4567;
                payload.beam1.resize(nRow);
                payload.beam1(1) = 123;
                payload.beam2.resize(nRow);
                payload.beam2(1) = 456;

                casa::Vector<casa::Stokes::StokesTypes> pols(nPol);
                pols(0) = casa::Stokes::XX;
                pols(1) = casa::Stokes::XY;
                pols(2) = casa::Stokes::YX;
                pols(3) = casa::Stokes::YY;
                payload.polarisations = pols;


                payload.vis.resize(nRow, nPol, nChannel);
                payload.nSamples.resize(nRow, nPol, nChannel);

                payload.nominalNSamples = 1000000;

                sendAndCompare(payload);
            }

        };

    }   // End namespace cp

}   // End namespace askap
