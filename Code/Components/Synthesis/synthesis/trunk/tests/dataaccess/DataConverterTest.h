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
   CPPUNIT_TEST(testFrequencyConversion);
   CPPUNIT_TEST(testVelocityConversion);
   CPPUNIT_TEST_EXCEPTION(testMissingRestFrequency1,std::exception);
   CPPUNIT_TEST_EXCEPTION(testMissingRestFrequency2,std::exception);
   CPPUNIT_TEST(testVelToFreq);
   CPPUNIT_TEST(testFreqToVel);
   CPPUNIT_TEST(testEpochToMeasures);
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
    casa::MEpoch refEpoch=casa::MEpoch(casa::MVEpoch(casa::Quantity(50257.29,"d")),
                            casa::MEpoch::Ref(casa::MEpoch::UTC));
    itsConverter->setEpochFrame(refEpoch,"s");
    CPPUNIT_ASSERT(fabs(itsConverter->epoch(refEpoch))<1e-7);
    // adjust it one day forward and see whether the epoch is converted
    // to seconds
    casa::MEpoch newEpoch=casa::MEpoch(casa::MVEpoch(casa::Quantity(50258.29,"d")),
                            casa::MEpoch::Ref(casa::MEpoch::UTC));
    CPPUNIT_ASSERT(fabs(itsConverter->epoch(newEpoch)-86400)<1e-7);

    // convert it to another frame and check again
    casa::MEpoch gmstEpoch=casa::MEpoch::Convert(newEpoch,
                         casa::MEpoch::Ref(casa::MEpoch::GMST))(newEpoch);    
    CPPUNIT_ASSERT(fabs(itsConverter->epoch(gmstEpoch)-86400)<1e-7);
 
    
    // convert it to LMST (which requires a position) and check again
    const casa::MeasFrame someFrame=getSomeFrame(WHERE_ONLY);
    // preserve only converted MVEpoch and instantiate MEpoch from scratch
    // in order to clean out the position and test converter properly
    casa::MEpoch lmstEpoch=casa::MEpoch(casa::MEpoch::Convert(newEpoch,
                  casa::MEpoch::Ref(casa::MEpoch::LMST,someFrame))(newEpoch).getValue(),
		  casa::MEpoch::LMST);    
    itsConverter->setMeasFrame(someFrame);
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
    const casa::MeasFrame someFrame=getSomeFrame(WHERE_AND_WHEN);
    casa::MDirection azelDir(casa::MDirection::Convert(galDir,
                           casa::MDirection::Ref(casa::MDirection::AZEL,
			   someFrame))(galDir));
    itsConverter->setMeasFrame(someFrame);		    
    itsConverter->direction(azelDir,result);
    CPPUNIT_ASSERT(result.separation(direction)<1e-7);
   }

   /// test Frequency conversion
   void testFrequencyConversion()
   {
     const casa::MVFrequency freq(casa::Quantity(1420,"MHz"));
     casa::MFrequency lsrkFreq(freq, casa::MFrequency::LSRK);
     itsConverter->setFrequencyFrame(casa::MFrequency::Ref(
                      casa::MFrequency::LSRK),"GHz");
		      
     CPPUNIT_ASSERT(itsConverter->isVoid(casa::MFrequency::Ref(
                      casa::MFrequency::LSRK),"GHz"));

     CPPUNIT_ASSERT(fabs(itsConverter->frequency(lsrkFreq)-1.42)<1e-7);

     // the same with topocentric (i.e. sky frequency) to LSRK conversion
     const casa::MeasFrame someFrame=getSomeFrame();
     
     casa::MFrequency topoFreq(casa::MFrequency::Convert(lsrkFreq,
                           casa::MFrequency::Ref(casa::MFrequency::TOPO,
			   someFrame))(lsrkFreq));
     itsConverter->setMeasFrame(someFrame);
     
     CPPUNIT_ASSERT(fabs(itsConverter->frequency(topoFreq)-1.42)<1e-5);
   }

   /// test Velocity conversion
   void testVelocityConversion()
   {
     const casa::MVRadialVelocity vel(casa::Quantity(-1000,"m/s"));
     casa::MRadialVelocity lsrkVel(vel, casa::MRadialVelocity::LSRK);
     itsConverter->setVelocityFrame(casa::MRadialVelocity::Ref(
                      casa::MRadialVelocity::LSRK),"km/s");
     
     CPPUNIT_ASSERT(fabs(itsConverter->velocity(lsrkVel)+1)<1e-7);

     // the same with topocentric (i.e. sky frequency) to LSRK conversion
     const casa::MeasFrame someFrame=getSomeFrame();

     casa::MRadialVelocity topoVel(casa::MRadialVelocity::Convert(lsrkVel,
                           casa::MRadialVelocity::Ref(casa::MRadialVelocity::TOPO,
			   someFrame))(lsrkVel));
     itsConverter->setMeasFrame(someFrame);

     CPPUNIT_ASSERT(fabs(itsConverter->velocity(topoVel)+1)<1e-7);
   }

   /// test missing rest frequency for velocity to frequency conversion
   void testMissingRestFrequency1() {     
     casa::MRadialVelocity lsrkVel((casa::MVRadialVelocity(casa::Quantity(
                                    -1000, "m/s"))),
				    casa::MRadialVelocity::LSRK);
     itsConverter->setFrequencyFrame(casa::MFrequency::Ref(
                                     casa::MFrequency::LSRK),"GHz");
     const casa::Double buf=itsConverter->frequency(lsrkVel);
     // to keep the compiler happy that this variable is used
     CONRADASSERT(true || buf==0);     
   }
   
   /// test missing rest frequency for frequency to velocity conversion
   void testMissingRestFrequency2() {
     casa::MFrequency lsrkFreq((casa::MVFrequency(casa::Quantity(
                                    1.4, "GHz"))),
				    casa::MFrequency::LSRK);
     itsConverter->setVelocityFrame(casa::MRadialVelocity::Ref(
                                    casa::MRadialVelocity::LSRK),"km/s");
     const casa::Double buf=itsConverter->velocity(lsrkFreq);     
     // to keep the compiler happy that this variable is used
     CONRADASSERT(true || buf==0);     
   }

   /// test velocity to frequency conversion
   void testVelToFreq() {     
     casa::MRadialVelocity lsrkVel((casa::MVRadialVelocity(casa::Quantity(
                                    -10, "km/s"))),
				    casa::MRadialVelocity::LSRK);
     itsConverter->setFrequencyFrame(casa::MFrequency::Ref(
                                     casa::MFrequency::TOPO),"MHz");
     itsConverter->setRestFrequency(casa::MVFrequency(casa::Quantity(
                                    1420.405752, "MHz")));
				    
     const casa::MeasFrame someFrame=getSomeFrame(FULL);     

     itsConverter->setMeasFrame(someFrame);     
          
     CPPUNIT_ASSERT(fabs(itsConverter->frequency(lsrkVel)-1420.464418)<1e-5);
   }

   /// test frequency to velocity conversion
   void testFreqToVel() {     
     casa::MFrequency topoFreq((casa::MVFrequency(casa::Quantity(
                                    1420464418, "Hz"))),
				    casa::MFrequency::TOPO);
     itsConverter->setVelocityFrame(casa::MRadialVelocity::Ref(
                                     casa::MRadialVelocity::LSRK),"km/s");
     itsConverter->setRestFrequency(casa::MVFrequency(casa::Quantity(
                                    1420.405752, "MHz")));
				    
     const casa::MeasFrame someFrame=getSomeFrame(FULL);     

     itsConverter->setMeasFrame(someFrame);
     
     CPPUNIT_ASSERT(fabs(itsConverter->velocity(topoFreq)+10)<1e-4);
   }

   /// test reverse "conversions" to measures for epoch
   void testEpochToMeasures() {
     casa::MEpoch refEpoch=casa::MEpoch(casa::MVEpoch(casa::Quantity(54257.29,"d")),
                            casa::MEpoch::Ref(casa::MEpoch::UTC));
     itsConverter->setEpochFrame(refEpoch,"d");
     casa::MEpoch newEpoch=casa::MEpoch(casa::MVEpoch(casa::Quantity(54258.29,"d")),
                            casa::MEpoch::Ref(casa::MEpoch::UTC));
     const casa::Double asDouble=itsConverter->epoch(newEpoch);
     const casa::MVEpoch asMVEpoch(casa::Quantity(asDouble,"d"));
     
     CPPUNIT_ASSERT(fabs(itsConverter->epoch(
              itsConverter->epochMeasure(asDouble))-1.)<1e-7);	      
     CPPUNIT_ASSERT(fabs(itsConverter->epoch(
              itsConverter->epochMeasure(asMVEpoch))-1.)<1e-7);     
   }
   
   
protected:

   /// a type of the frame requested
   enum FrameType {
      WHERE_ONLY,
      WHERE_AND_WHEN,
      FULL
   };
   /// an auxiliary static method to construct an arbitrary frame, where
   /// the conversion is performed.
   /// @param type a FrameType enum describing which aspects of the frame
   ///             must be set
   static casa::MeasFrame getSomeFrame(FrameType type = FULL) {
     const casa::MPosition where((casa::MVPosition(casa::Quantity(267.,"m"),
                                   casa::Quantity(149.549,"deg"),
		                   casa::Quantity(-30.2644,"deg"))),
				   casa::MPosition::WGS84);

     if (type==WHERE_ONLY) {
         return casa::MeasFrame(where);
     }

     casa::MEpoch when=casa::MEpoch(casa::MVEpoch(casa::Quantity(54255.29,"d")),
                            casa::MEpoch::Ref(casa::MEpoch::UTC));

     if (type==WHERE_AND_WHEN) {
         return casa::MeasFrame(where,when);
     }
    
     casa::MDirection what((casa::MVDirection(casa::Quantity(30.,"deg"),
                                 casa::Quantity(-50.,"deg"))),
				 casa::MDirection::J2000);

     return casa::MeasFrame(where,when,what);
   }
private:
   boost::shared_ptr<BasicDataConverter> itsConverter;
};

} // namespace synthesis
} // namespace conrad

#endif // #ifndef I_DATA_CONVERTER_TEST_IMPL_H
