/// @file IceMapperTest.h
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

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include "casa/aipstype.h"
#include "casa/BasicSL/Complex.h"
#include "CalibrationDataService.h" // Ice generated interface
#include "calibrationclient/JonesJTerm.h"
#include "calibrationclient/JonesIndex.h"
#include "calibrationclient/GenericSolution.h"

// Classes to test
#include "calibrationclient/IceMapper.h"

namespace askap {
namespace cp {
namespace caldataservice {

class IceMapperTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(IceMapperTest);
        CPPUNIT_TEST(testGainToIce);
        CPPUNIT_TEST(testLeakageToIce);
        CPPUNIT_TEST(testBandpassToIce);
        CPPUNIT_TEST(testGainFromIce);
        CPPUNIT_TEST(testLeakageFromIce);
        CPPUNIT_TEST(testBandpassFromIce);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
        };

        void tearDown()
        {
        }

        void testGainToIce()
        {
            // Build the source object
            askap::cp::caldataservice::GainSolution sol(theirTimestamp);

            casa::Double val = 1.0;
            for (casa::Short antenna = 1; antenna <= theirNAntenna; ++antenna) {
                for (casa::Short beam = 1; beam <= theirNBeam; ++beam) {
                    JonesJTerm jterm(casa::DComplex(val, val), true,
                            casa::DComplex(0.0, 0.0), false);
                    sol.map()[JonesIndex(antenna, beam)] = jterm;
                    val += 0.1;
                }
            }

            // Convert
            const askap::interfaces::calparams::TimeTaggedGainSolution ice_sol = IceMapper::toIce(sol);

            // Compare source with the target copy
            CPPUNIT_ASSERT_EQUAL(static_cast<long>(theirTimestamp), static_cast<long>(ice_sol.timestamp));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(theirNAntenna * theirNBeam), ice_sol.solutionMap.size());
            val = 1.0;
            for (casa::Short antenna = 1; antenna <= theirNAntenna; ++antenna) {
                for (casa::Short beam = 1; beam <= theirNBeam; ++beam) {
                    askap::interfaces::calparams::JonesIndex jindex;
                    jindex.antennaID = antenna;
                    jindex.beamID = beam;
                    askap::interfaces::calparams::JonesJTerm actual = ice_sol.solutionMap.find(jindex)->second;

                    CPPUNIT_ASSERT_EQUAL(true, actual.g1Valid);
                    CPPUNIT_ASSERT_EQUAL(false, actual.g2Valid);
                    askap::interfaces::DoubleComplex expected;
                    expected.real = val;
                    expected.imag = val;
                    compare(expected, actual.g1);

                    val += 0.1;
                }
            }
        }

        void testLeakageToIce()
        {
            // Build the source object
            askap::cp::caldataservice::LeakageSolution sol(theirTimestamp);

            // Convert
            const askap::interfaces::calparams::TimeTaggedLeakageSolution ice_sol = IceMapper::toIce(sol);

            // Compare source with the target copy
            CPPUNIT_ASSERT_EQUAL(static_cast<long>(theirTimestamp), static_cast<long>(ice_sol.timestamp));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), ice_sol.solutionMap.size());
        }

        void testBandpassToIce()
        {
            // Build the source object
            askap::cp::caldataservice::BandpassSolution sol(theirTimestamp);

            // Convert
            const askap::interfaces::calparams::TimeTaggedBandpassSolution ice_sol = IceMapper::toIce(sol);

            // Compare source with the target copy
            CPPUNIT_ASSERT_EQUAL(static_cast<long>(theirTimestamp), static_cast<long>(ice_sol.timestamp));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), ice_sol.solutionMap.size());
        }

        void testGainFromIce()
        {
            // Build the source object
            askap::interfaces::calparams::TimeTaggedGainSolution ice_sol;
            ice_sol.timestamp = theirTimestamp;

            double val = 1.0;
            for (casa::Short antenna = 1; antenna <= theirNAntenna; ++antenna) {
                for (casa::Short beam = 1; beam <= theirNBeam; ++beam) {
                    askap::interfaces::calparams::JonesJTerm jterm;

                    askap::interfaces::DoubleComplex g1;
                    g1.real = val;
                    g1.imag = val;
                    jterm.g1 = g1;
                    jterm.g1Valid = true;
                    askap::interfaces::DoubleComplex g2;
                    g2.real = 0.0;
                    g2.imag = 0.0;
                    jterm.g2 = g2;
                    jterm.g2Valid = false;

                    askap::interfaces::calparams::JonesIndex jindex;
                    jindex.antennaID = antenna;
                    jindex.beamID = beam;
                    ice_sol.solutionMap[jindex] = jterm;
                    val += 0.1;
                }
            }

            // Convert
            const askap::cp::caldataservice::GainSolution sol = IceMapper::fromIce(ice_sol);

            // Compare source with the target copy
            CPPUNIT_ASSERT_EQUAL(theirTimestamp, sol.timestamp());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(theirNAntenna * theirNBeam), sol.map().size());
            val = 1.0;
            for (casa::Short antenna = 1; antenna <= theirNAntenna; ++antenna) {
                for (casa::Short beam = 1; beam <= theirNBeam; ++beam) {
                    JonesJTerm actual = sol.map().find(JonesIndex(antenna, beam))->second;

                    CPPUNIT_ASSERT_EQUAL(true, actual.g1IsValid());
                    CPPUNIT_ASSERT_EQUAL(false, actual.g2IsValid());
                    compare(casa::DComplex(val, val), actual.g1());
                    val += 0.1;
                }
            }
        }

        void testLeakageFromIce()
        {
            // Build the source object
            askap::interfaces::calparams::TimeTaggedLeakageSolution ice_sol;
            ice_sol.timestamp = theirTimestamp;

            // Convert
            const askap::cp::caldataservice::LeakageSolution sol = IceMapper::fromIce(ice_sol);

            // Compare source with the target copy
            CPPUNIT_ASSERT_EQUAL(theirTimestamp, sol.timestamp());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), sol.map().size());
        }

        void testBandpassFromIce()
        {
            // Build the source object
            askap::interfaces::calparams::TimeTaggedBandpassSolution ice_sol;
            ice_sol.timestamp = theirTimestamp;

            // Convert
            const askap::cp::caldataservice::BandpassSolution sol = IceMapper::fromIce(ice_sol);

            // Compare source with the target copy
            CPPUNIT_ASSERT_EQUAL(theirTimestamp, sol.timestamp());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), sol.map().size());
        }

    private:

        void compare(const casa::DComplex& expected, const casa::DComplex& actual)
        {
            const int tolerance = 0.000001;
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.real(), actual.real(), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.imag(), actual.imag(), tolerance);
        }

        void compare(const askap::interfaces::DoubleComplex& expected,
                const askap::interfaces::DoubleComplex& actual)
        {
            const int tolerance = 0.000001;
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.real, actual.real, tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.imag, actual.imag, tolerance);
        }
 
        static const casa::Long theirTimestamp;
        static const casa::Short theirNAntenna;
        static const casa::Short theirNBeam;
};

const casa::Long IceMapperTest::theirTimestamp = 1234567890;
const casa::Short IceMapperTest::theirNAntenna = 36;
const casa::Short IceMapperTest::theirNBeam = 32;

}   // End namespace caldataservice
}   // End namespace cp
}   // End namespace askap
