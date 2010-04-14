/// @file MetadataConverterReverseTest.cc
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

// System includes
#include <string>
#include <sstream>
#include <vector>

// Support classes
#include "boost/scoped_ptr.hpp"
#include "TypedValues.h"
#include "cpcommon/TosMetadata.h"

// Classes to test
#include "tosmetadata/MetadataConverter.h"

// Using
using namespace casa;
using namespace askap::interfaces;

namespace askap {
    namespace cp {

        class MetadataConverterReverseTest : public CppUnit::TestFixture {
            CPPUNIT_TEST_SUITE(MetadataConverterReverseTest);
            CPPUNIT_TEST(testTime);
            CPPUNIT_TEST(testPeriod);
            CPPUNIT_TEST(testNBeams);
            CPPUNIT_TEST(testNCoarseChan);
            CPPUNIT_TEST(testNAntennas);
            CPPUNIT_TEST(testNPol);
            CPPUNIT_TEST(testAntennaNames);
            CPPUNIT_TEST_SUITE_END();

            public:
            void setUp() {
                // Initialize test values
                nCoarseChan = 304;
                nBeam = 36;
                nPol = 4;
                nAntenna = 1;
                timestamp = 1234567890;
                period = 5 * 1000 * 1000;

                // Setup the source object
                itsSource.reset(new TimeTaggedTypedValueMap);

                // time
                itsSource->data["time"] = new TypedValueLong(TypeLong, timestamp);

                // period
                itsSource->data["period"] = new TypedValueLong(TypeLong, period);

                // n_coarse_chan
                itsSource->data["n_coarse_chan"] = new TypedValueInt(TypeInt, nCoarseChan);

                // n_antennas
                itsSource->data["n_antennas"] = new TypedValueInt(TypeInt, nAntenna);

                // n_beams
                {
                    IntSeq iseq;
                    iseq.assign(nCoarseChan, nBeam);
                    TypedValueIntSeqPtr tv = new TypedValueIntSeq(TypeIntSeq, iseq);
                    itsSource->data["n_beams"] = tv;
                }

                // n_pol
                itsSource->data["n_pol"] = new TypedValueInt(TypeInt, nPol);


                // Convert
                MetadataConverter converter;
                itsDest.reset(new TosMetadata(converter.convert(*itsSource)));
            }

            void tearDown() {
                itsSource.reset();
                itsDest.reset();
            }

            void testTime() {
            }

            void testPeriod() {
            }

            void testNBeams() {
            }

            void testNCoarseChan() {
            }

            void testNAntennas() {
            }

            void testNPol() {
            }

            void testAntennaNames() {
            }


            private:
            // Support classes
            boost::scoped_ptr<TimeTaggedTypedValueMap> itsSource;
            boost::scoped_ptr<TosMetadata> itsDest;

            // Some constants
            casa::Int nCoarseChan;
            casa::Int nBeam;
            casa::Int nPol;
            casa::Int nAntenna;
            casa::Long timestamp;
            casa::Long period;
        };

    }   // End namespace cp

}   // End namespace askap
