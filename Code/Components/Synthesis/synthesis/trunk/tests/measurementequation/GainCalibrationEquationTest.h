/// @file
/// 
/// @brief Unit tests for GainCalibrationEquation.
/// @details GainCalibrationEquation just multiplies by a gain matrix
/// visibilities produced by another measurement equation. It also generates
/// normal equations, which allow to solve for unknowns in the gain matrix.
/// The tests given in this file attempt to predict a visibility data set
/// with some calibration errors and then solve for them.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef GAIN_CALIBRATION_EQUATION_TEST_H
#define GAIN_CALIBRATION_EQUATION_TEST_H

#include <measurementequation/ComponentEquation.h>
#include <measurementequation/GainCalibrationEquation.h>
#include <fitting/LinearSolver.h>
#include <dataaccess/DataIteratorStub.h>
#include <cppunit/extensions/HelperMacros.h>

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>

#include <boost/shared_ptr.hpp>


namespace conrad
{
  namespace synthesis
  {
    using utility::toString;
    
    class GainCalibrationEquationTest : public CppUnit::TestFixture
    {
      CPPUNIT_TEST_SUITE(GainCalibrationEquationTest);
      CPPUNIT_TEST(testSolve);
      CPPUNIT_TEST_SUITE_END();
      
      private:
        boost::shared_ptr<ComponentEquation> p1, p2;
        boost::shared_ptr<GainCalibrationEquation> eq1,eq2;
        boost::shared_ptr<Params> params1, params2;
        IDataSharedIter idi;

      public:
        void setUp()
        {
          idi = IDataSharedIter(new DataIteratorStub(1));
          const casa::uInt nAnt = 30;
          const casa::uInt nAnt1 = 6;
          const double realGains[nAnt] = {1.1, 0.9, 1.05, 0.87, 1.333,
                                          1.1, 1.0, 1.0, -1.0, 0.3, 
                                         -0.5, 1.1, 0.9, 0.98, 1.03,
                                         -0.3, -1.1, 0.9, 1.1, 1.05,
                                          1.0, -0.3, 1.1, 0.3, 1.8,
                                          0.5, -0.7, 1.054, 1.0, 1.1}; 
          const double imagGains[nAnt] = {0.0, 0., -0.05, 0.587, 0.,
                                          0., -0.1, 0.02, -0.1, 0.84, 
                                          0.86, 0.1, 0.1, 0., 0.03,
                                         -0.84, 0., 0., -0.1, -0.05,
                                          0.2, 0.9, 1.1, 0.3, -0.1,
                                         -0.9, 0.72, -0.04, 0.05, -0.1}; 
          
          params1.reset(new Params);
          params1->add("flux.i.cena", 100.);
          params1->add("direction.ra.cena", 0.5);
          params1->add("direction.dec.cena", -0.3);
          params1->add("shape.bmaj.cena", 30.0*casa::C::arcsec);
          params1->add("shape.bmin.cena", 20.0*casa::C::arcsec);
          params1->add("shape.bpa.cena", -55*casa::C::degree);
          for (casa::uInt ant=0; ant<nAnt; ++ant) {
               params1->add("gain.g11."+toString(ant),
                            casa::Complex(realGains[ant],imagGains[ant]));
               params1->add("gain.g22."+toString(ant),1.);
          }

          p1.reset(new ComponentEquation(*params1, idi));
          eq1.reset(new GainCalibrationEquation(*params1,idi,*p1));

          params2.reset(new Params);
          params2->add("flux.i.cena", 100.);
          params2->add("direction.ra.cena", 0.50000);
          params2->add("direction.dec.cena", -0.30000);
          params2->add("shape.bmaj.cena", 30.0*casa::C::arcsec);
          params2->add("shape.bmin.cena", 20.0*casa::C::arcsec);
          params2->add("shape.bpa.cena", -55*casa::C::degree);
          for (casa::uInt ant=0; ant<nAnt; ++ant) {
               params2->add("gain.g11."+toString(ant),casa::Complex(1.0,0.0));
               params2->add("gain.g22."+toString(ant),1.0);
               params2->fix("gain.g22."+toString(ant));
          }
       
          p2.reset(new ComponentEquation(*params2, idi));
          eq2.reset(new GainCalibrationEquation(*params2,idi,*p2));

        }

        void testSolve()
        {
          // Predict with the "perfect" parameters"
          eq1->predict();
          std::vector<std::string> freeNames = params2->freeNames();
          for (std::vector<std::string>::const_iterator it = freeNames.begin();
               it!=freeNames.end();++it) {
               if (it->find("gain") != 0) {
                   params2->fix(*it);
               }
          }
          for (size_t iter=0; iter<5; ++iter) {
               // Calculate gradients using "imperfect" parameters"
               NormalEquations ne(*params2);
               eq2->calcEquations(ne);
               Quality q;
               LinearSolver solver1(*params2);
               solver1.addNormalEquations(ne);
               solver1.setAlgorithm("SVD");
               solver1.solveNormalEquations(q);  
               
               // taking care of the absolute phase uncertainty
               const casa::uInt refAnt = 0;
               const casa::Complex refPhaseTerm = casa::polar(1.f,
                       -arg(params2->complexValue("gain.g11."+toString(refAnt))));
                       
               std::vector<std::string> freeNames(params2->freeNames());
               for (std::vector<std::string>::const_iterator it=freeNames.begin();
                                                   it!=freeNames.end();++it)  {
                    const std::string parname = *it;
                    if (parname.find("gain") == 0) {                    
                        params2->update(parname,
                             params2->complexValue(parname)*refPhaseTerm);                                 
                    } 
               }
               
          }
          //std::cout<<*params2<<std::endl;
        
          // checking that solved gains should be close to 1 for g11 
          // and to 0.9 for g22 (we don't have data to solve for the second
          // polarisation, so it should be left unchanged)
          CONRADASSERT(params2);
          std::vector<std::string> completions(params2->completions("gain"));
          for (std::vector<std::string>::const_iterator it=completions.begin();
                                                it!=completions.end();++it)  {
               const std::string parname = "gain"+*it;                                 
               
               if (it->find(".g22") == 0) {
                   CPPUNIT_ASSERT(fabs(params2->scalarValue(parname)-1.0)<1e-7);
               } else if (it->find(".g11") == 0) {
                   CPPUNIT_ASSERT(abs(params2->complexValue(parname)-
                          params1->complexValue(parname))<1e-7);
               } else {
                 CONRADTHROW(ConradError, "an invalid gain parameter "<<parname<<" has been detected");
               }
          }
        }
   };
    
  } // namespace synthesis
} // namespace conrad

#endif // #ifndef GAIN_CALIBRATION_EQUATION_TEST_H

