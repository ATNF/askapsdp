/// @file MetadataConverterTest.cc
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
#include "askap/AskapError.h"
#include "boost/scoped_ptr.hpp"
#include "TypedValues.h"
#include "cpcommon/TosMetadata.h"
#include "casa/aips.h"
#include "measures/Measures/MDirection.h"

// Classes to test
#include "tosmetadata/MetadataConverter.h"

// Using
using namespace casa;
using namespace askap::interfaces;

namespace askap {
namespace cp {
namespace icewrapper {

class MetadataConverterTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(MetadataConverterTest);
        CPPUNIT_TEST(testNonLP64);
#ifdef __LP64__
        CPPUNIT_TEST(testConverter);
        CPPUNIT_TEST(testConverterAntenna);
#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            // Test values (TosMetadata)
            const casa::uInt nCoarseChan = 304;
            const casa::uInt nBeam = 36;
            const casa::uInt nPol = 4;
            const casa::uInt nAntenna = 6;
            const casa::uLong timestamp = 1234567890;
            const casa::uLong period = 5 * 1000 * 1000;

            // Test values (TosMetadataAntenna)
            const casa::Double frequency = 1.4 * 1000000;
            const MDirection testDir(Quantity(20, "deg"),
                                     Quantity(-10, "deg"),
                                     MDirection::Ref(MDirection::J2000));
            const std::string clientId = "testClient";
            const std::string scanId = "testScan";
            const casa::Double polarisationOffset = 1.234567;
            const casa::Bool flag = false;
            const casa::Bool onSource = true;
            const casa::Bool hwError = false;
            const casa::Float systemTemp = 50.0;

            //////////////////////////////////////
            // Setup the source TosMetadata object
            //////////////////////////////////////
            itsSource.reset(new TosMetadata(nCoarseChan, nBeam, nPol));

            // Time
            itsSource->time(timestamp);

            // Period
            itsSource->period(period);

            // Antennas
            std::vector<std::string> antennaNames;

            for (casa::uInt i = 0; i < nAntenna; ++i) {
                std::stringstream ss;
                ss << "ASKAP" << i;
                casa::uInt id = itsSource->addAntenna(ss.str());
                antennaNames.push_back(ss.str());
                TosMetadataAntenna& ant = itsSource->antenna(id);

                ant.targetRaDec(testDir);
                ant.frequency(frequency);
                ant.clientId(clientId);
                ant.scanId(scanId);
                ant.polarisationOffset(polarisationOffset);
                ant.onSource(onSource);
                ant.hwError(hwError);

                for (casa::uInt beam = 0; beam < nBeam; ++beam) {
                    for (casa::uInt coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
                        ant.phaseTrackingCentre(testDir, beam, coarseChan);

                        for (casa::uInt pol = 0; pol < nPol; ++pol) {
                            ant.flagDetailed(flag, beam, coarseChan, pol);
                            ant.systemTemp(systemTemp, beam, coarseChan, pol);
                        }
                    }
                }

            }

#ifndef __LP64__

            try {
#endif
                ///////////////////////////////////////////////////
                // Convert (TosMetadata -> TimeTaggedTypedValueMap)
                ///////////////////////////////////////////////////
                MetadataConverter converter;
                TimeTaggedTypedValueMap intermediate(converter.convert(*itsSource));

                ////////////////////////////////
                // Convert back to a TosMetadata
                ////////////////////////////////
                itsResult.reset(new TosMetadata(converter.convert(intermediate)));
#ifndef __LP64__
                CPPUNIT_FAIL("Expected exception not thrown");
            } catch (askap::AskapError&) {
                // This is expected for 32-bit platforms
            }

#endif
        }

        void tearDown() {
            itsSource.reset();
            itsResult.reset();
        }

        void testNonLP64() {
            // This is here to ensure setUp() is attempted on 32-bit platforms.
            // The other tests are for LP64 platforms only.
        }

        void testConverter() {
            CPPUNIT_ASSERT_EQUAL(itsSource->nAntenna(), itsResult->nAntenna());
            CPPUNIT_ASSERT_EQUAL(itsSource->nCoarseChannels(), itsResult->nCoarseChannels());
            CPPUNIT_ASSERT_EQUAL(itsSource->nBeams(), itsResult->nBeams());
            CPPUNIT_ASSERT_EQUAL(itsSource->nPol(), itsResult->nPol());
            CPPUNIT_ASSERT_EQUAL(itsSource->time(), itsResult->time());
            CPPUNIT_ASSERT_EQUAL(itsSource->period(), itsResult->period());
        }

        void testConverterAntenna() {
            const unsigned int nAntenna = itsSource->nAntenna();
            CPPUNIT_ASSERT_EQUAL(nAntenna, itsResult->nAntenna());

            for (unsigned int i = 0; i < nAntenna; ++i) {
                verifyAntenna(i);
            }
        }

    private:

        void verifyAntenna(unsigned int id) {
            // Get the TosMetadataAntenna instance
            TosMetadataAntenna& srcAnt = itsSource->antenna(id);
            TosMetadataAntenna& resultAnt = itsResult->antenna(id);

            const casa::uInt nCoarseChan = srcAnt.nCoarseChannels();
            const casa::uInt nBeam = srcAnt.nBeams();
            const casa::uInt nPol = srcAnt.nPol();

            CPPUNIT_ASSERT_EQUAL(srcAnt.name(), resultAnt.name());
            CPPUNIT_ASSERT_EQUAL(srcAnt.nCoarseChannels(),
                                 resultAnt.nCoarseChannels());
            CPPUNIT_ASSERT_EQUAL(srcAnt.nBeams(), resultAnt.nBeams());
            CPPUNIT_ASSERT_EQUAL(srcAnt.nPol(), resultAnt.nPol());

            verifyDir(srcAnt.targetRaDec(), resultAnt.targetRaDec());
            CPPUNIT_ASSERT_EQUAL(srcAnt.frequency(), resultAnt.frequency());
            CPPUNIT_ASSERT_EQUAL(srcAnt.clientId(), resultAnt.clientId());
            CPPUNIT_ASSERT_EQUAL(srcAnt.scanId(), resultAnt.scanId());
            CPPUNIT_ASSERT_EQUAL(srcAnt.polarisationOffset(),
                                 resultAnt.polarisationOffset());
            CPPUNIT_ASSERT_EQUAL(srcAnt.onSource(), resultAnt.onSource());
            CPPUNIT_ASSERT_EQUAL(srcAnt.hwError(), resultAnt.hwError());

            // Check the per beam, per coarse channel and per polarisations
            // data
            for (casa::uInt beam = 0; beam < nBeam; ++beam) {
                for (casa::uInt coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {

                    // phaseTrackingCentre
                    verifyDir(srcAnt.phaseTrackingCentre(beam, coarseChan),
                              resultAnt.phaseTrackingCentre(beam, coarseChan));

                    for (casa::uInt pol = 0; pol < nPol; ++pol) {
                        // flagDetailed
                        CPPUNIT_ASSERT_EQUAL(srcAnt.flagDetailed(beam, coarseChan, pol),
                                             resultAnt.flagDetailed(beam, coarseChan, pol));

                        // systemTemp
                        CPPUNIT_ASSERT_EQUAL(srcAnt.systemTemp(beam, coarseChan, pol),
                                             resultAnt.systemTemp(beam, coarseChan, pol));
                    }
                }
            }
        }

        void verifyDir(const casa::MDirection& d1, const casa::MDirection& d2) {
            CPPUNIT_ASSERT_EQUAL(d1.getAngle().getValue()(0), d2.getAngle().getValue()(0));
            CPPUNIT_ASSERT_EQUAL(d1.getAngle().getValue()(1), d2.getAngle().getValue()(1));
            CPPUNIT_ASSERT(d1.getRef().getType() == d2.getRef().getType());
        }

        // Support classes
        boost::scoped_ptr<TosMetadata> itsSource;
        boost::scoped_ptr<TosMetadata> itsResult;

        // Some constants
};

}   // End namespace icewrapper

}   // End namespace cp

}   // End namespace askap
