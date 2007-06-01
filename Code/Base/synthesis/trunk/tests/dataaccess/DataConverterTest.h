/// @file DataConverterTest.h
///
/// DataConverterTest: Tests of the DataConverter class(es)
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 
#ifndef I_DATA_CONVERTER_TEST_IMPL_H
#define I_DATA_CONVERTER_TEST_IMPL_H

#include<iostream>
#include<stdexcept>
using namespace std;

// boost includes
#include <boost/shared_ptr.hpp>

// CASA includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MCDirection.h>
#include <casa/Quanta/MVPosition.h>

// own includes
#include <dataaccess/BasicDataConverter.h>

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>

namespace conrad {

namespace synthesis {

class DataConverterTest : public CppUnit::TestFixture {

   CPPUNIT_TEST_SUITE(DataConverterTest);
   CPPUNIT_TEST(testEpochConversion);
   CPPUNIT_TEST(testDirectionConversion);
   CPPUNIT_TEST_EXCEPTION(testMissingFrame,std::exception);
   CPPUNIT_TEST_SUITE_END();
public:
   void setUp()
   {
     itsConverter.reset(new BasicDataConverter);
   }
   void tearDown()
   {
     itsConverter.reset();
   }
   /// test Epoch conversion
   void testEpochConversion()
   { 
    casa::MEpoch refEpoch=casa::MEpoch(casa::MVEpoch(casa::Quantity(50237.29,"d")),
                            casa::MEpoch::Ref(casa::MEpoch::UTC));
    itsConverter->setEpochFrame(refEpoch,"s");
    CPPUNIT_ASSERT(fabs(itsConverter->epoch(refEpoch))<1e-7);
    // adjust it one day forward and see whether the epoch is converted
    // to seconds
    casa::MEpoch newEpoch=casa::MEpoch(casa::MVEpoch(casa::Quantity(50238.29,"d")),
                            casa::MEpoch::Ref(casa::MEpoch::UTC));
    CPPUNIT_ASSERT(fabs(itsConverter->epoch(newEpoch)-86400)<1e-7);

    // convert it to another frame and check again
    casa::MEpoch gmstEpoch=casa::MEpoch::Convert(newEpoch,
                         casa::MEpoch::Ref(casa::MEpoch::GMST))(newEpoch);    
    CPPUNIT_ASSERT(fabs(itsConverter->epoch(gmstEpoch)-86400)<1e-7);
 
    
    // convert it to LMST (which requires a position) and check again
    const casa::MVPosition  wgsLoc(casa::Quantity(25.,"m"),
                                   casa::Quantity(145.,"deg"),
		                   casa::Quantity(-33.,"deg"));
    
    const casa::MPosition where(wgsLoc, casa::MPosition::WGS84);

    // preserve only converted MVEpoch and instantiate MEpoch from scratch
    // in order to clean out the position and test converter properly
    casa::MEpoch lmstEpoch=casa::MEpoch(casa::MEpoch::Convert(newEpoch,
                  casa::MEpoch::Ref(casa::MEpoch::LMST,where))(newEpoch).getValue(),
		  casa::MEpoch::LMST);    
    itsConverter->setMeasFrame(where);		  
    CPPUNIT_ASSERT(fabs(itsConverter->epoch(lmstEpoch)-86400)<1e-7);
    
   }

   void testMissingFrame()
   {
     casa::MDirection theSun(casa::MDirection::SUN);
     itsConverter->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
     casa::MVDirection result;
     itsConverter->direction(theSun,result);
   }

   /// test Direction conversion
   void testDirectionConversion()
   {
    
    const casa::MVDirection direction(casa::Quantity(30.,"deg"),
                                  casa::Quantity(-50.,"deg"));
    casa::MDirection j2000Dir(direction, casa::MDirection::J2000);
    casa::MDirection galDir(casa::MDirection::Convert(j2000Dir,
                           casa::MDirection::GALACTIC)(j2000Dir));

    itsConverter->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
    casa::MVDirection result;
    itsConverter->direction(galDir,result);
    CPPUNIT_ASSERT(result.separation(direction)<1e-7);

    // convert direction to Az/El (which requires time and position)
    // and check again
    const casa::MVPosition  wgsLoc(casa::Quantity(25.,"m"),
                                   casa::Quantity(145.,"deg"),
		                   casa::Quantity(-33.,"deg"));
    
    const casa::MPosition where(wgsLoc, casa::MPosition::WGS84);

    casa::MEpoch when=casa::MEpoch(casa::MVEpoch(casa::Quantity(50237.29,"d")),
                            casa::MEpoch::Ref(casa::MEpoch::UTC));
    casa::MDirection azelDir(casa::MDirection::Convert(galDir,
                           casa::MDirection::Ref(casa::MDirection::AZEL,
			   casa::MeasFrame(where,when)))(galDir));
    itsConverter->setMeasFrame(casa::MeasFrame(where,when));		    
    itsConverter->direction(azelDir,result);
    CPPUNIT_ASSERT(result.separation(direction)<1e-7);
   }
private:
   boost::shared_ptr<BasicDataConverter> itsConverter;
};

} // namespace synthesis
} // namespace conrad

#endif // #ifndef I_DATA_CONVERTER_TEST_IMPL_H
