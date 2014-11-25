/// @file SpectralIndexTest.cc
///
/// @copyright (c) 2011 CSIRO
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

// System includes
#include <limits>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include "measures/Measures/MFrequency.h"

// Classes to test
#include "components/SpectralIndex.h"

namespace askap {
namespace components {

class SpectralIndexTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(SpectralIndexTest);
        CPPUNIT_TEST(testConstructorInvalidFreq);
        CPPUNIT_TEST(testType);
        CPPUNIT_TEST(testSample);
        CPPUNIT_TEST(testSampleInvalidArguments);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }

        void testConstructorInvalidFreq() {
            const casa::MFrequency zero(casa::Quantity(0.0, "Hz"));
            CPPUNIT_ASSERT_THROW(SpectralIndex(zero, 0.5), AskapError);

            const casa::MFrequency negative(casa::Quantity(-10.0, "Hz"));
            CPPUNIT_ASSERT_THROW(SpectralIndex(negative, 0.5), AskapError);
        }

        void testType() {
            const casa::MFrequency freq(casa::Quantity(1400, "MHz"));
            const SpectralIndex instance(freq, 0.5);
            CPPUNIT_ASSERT_EQUAL(ComponentType::SPECTRAL_INDEX, instance.type());
        }

        void testSample() {
            const casa::MFrequency refFreq(casa::Quantity(1400, "MHz"));
            const casa::MFrequency userFreq(casa::Quantity(850, "Hz"));

            const SpectralIndex positive(refFreq, 0.05);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.48883753, positive.sample(userFreq), 10e-6);

            const SpectralIndex constant(refFreq, 0.0);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, constant.sample(userFreq), 10e-6);

            const SpectralIndex negative(refFreq, -0.05);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0456694, negative.sample(userFreq), 10e-6);
        }

        void testSampleInvalidArguments() {
            const casa::MFrequency refFreq(casa::Quantity(1400, "MHz"));
            const SpectralIndex instance(refFreq, 0.5);

            const casa::MFrequency zero(casa::Quantity(0.0, "Hz"));
            CPPUNIT_ASSERT_THROW(instance.sample(zero), AskapError);

            const casa::MFrequency negative(casa::Quantity(-10.0, "Hz"));
            CPPUNIT_ASSERT_THROW(instance.sample(negative), AskapError);
        }
};

}   // End namespace components
}   // End namespace askap
