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
using namespace std;

// boost includes
#include <boost/shared_ptr.hpp>

// CASA includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MCEpoch.h>
#include <casa/Quanta/MVPosition.h>

// own includes
#include <dataaccess/BasicDataConverter.h>

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>

namespace conrad {

namespace synthesis {

class DataConverterTest : public CppUnit::TestFixture {

   CPPUNIT_TEST_SUITE(DataConverterTest);
   CPPUNIT_TEST(testConversion);
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
   void testConversion()
   {
     checkEpochConversion();
   }
protected:
   /// test Epoch conversion
   void checkEpochConversion()
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
private:
   boost::shared_ptr<BasicDataConverter> itsConverter;
};

} // namespace synthesis
} // namespace conrad

#endif // #ifndef I_DATA_CONVERTER_TEST_IMPL_H
