/// @file CasaBlobUtilsTest.cc
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
#include "Blob/BlobIStream.h"
#include "Blob/BlobIBufVector.h"
#include "Blob/BlobOStream.h"
#include "Blob/BlobOBufVector.h"

// Classes to test
#include "blobutils/CasaBlobUtils.h"

using namespace casa;

namespace askap {
namespace cp {

class CasaBlobUtils : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(CasaBlobUtils);
        CPPUNIT_TEST(testMVEpoch);
        CPPUNIT_TEST(testMDirection);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
        };

        void tearDown()
        {
        }

        void testMVEpoch()
        {
            casa::MVEpoch source(casa::Quantity((1000000.0), "s"));
            casa::MVEpoch target;

            // Encode
            std::vector<int8_t> buf;
            LOFAR::BlobOBufVector<int8_t> obv(buf);
            LOFAR::BlobOStream out(obv);
            out.putStart("MVEpoch", 1);
            out << source;
            out.putEnd();

            // Decode
            LOFAR::BlobIBufVector<int8_t> ibv(buf);
            LOFAR::BlobIStream in(ibv);
            int version = in.getStart("MVEpoch");
            ASKAPASSERT(version == 1);
            in >> target;
            in.getEnd();

            CPPUNIT_ASSERT_EQUAL(source, target);
        }

        void testMDirection()
        {
            casa::MDirection source(casa::Quantity(123.0, "rad"),
                    casa::Quantity(456.0, "rad"),
                    casa::MDirection::Ref(casa::MDirection::B1950));
            casa::MDirection target;

            // Encode
            std::vector<int8_t> buf;
            LOFAR::BlobOBufVector<int8_t> obv(buf);
            LOFAR::BlobOStream out(obv);
            out.putStart("MDirection", 1);
            out << source;
            out.putEnd();

            // Decode
            LOFAR::BlobIBufVector<int8_t> ibv(buf);
            LOFAR::BlobIStream in(ibv);
            int version = in.getStart("MDirection");
            ASKAPASSERT(version == 1);
            in >> target;
            in.getEnd();

            // Tolerance for double equality
            const double tol = 1.0E-8;
            CPPUNIT_ASSERT_DOUBLES_EQUAL(source.getAngle().getValue()(0),
                    target.getAngle().getValue()(0), tol);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(source.getAngle().getValue()(1),
                    target.getAngle().getValue()(1), tol);
            CPPUNIT_ASSERT_EQUAL(source.getRefString(), target.getRefString());
        }
};

}   // End namespace cp

}   // End namespace askap
