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

          params1.reset(new Params);
          params1->add("flux.i.cena", 100.0);
          params1->add("direction.ra.cena", 0.5);
          params1->add("direction.dec.cena", -0.3);
          params1->add("shape.bmaj.cena", 30.0*casa::C::arcsec);
          params1->add("shape.bmin.cena", 20.0*casa::C::arcsec);
          params1->add("shape.bpa.cena", -55*casa::C::degree);
          for (casa::uInt ant=0; ant<nAnt; ++ant) {
               params1->add("gain.g11."+toString(ant),1.);
               params1->add("gain.g22."+toString(ant),1.);
          }

          p1.reset(new ComponentEquation(*params1, idi));
          eq1.reset(new GainCalibrationEquation(*params1,idi,*p1));

          params2.reset(new Params);
          params2->add("flux.i.cena", 100.0);
          params2->add("direction.ra.cena", 0.500005);
          params2->add("direction.dec.cena", -0.300003);
          params2->add("shape.bmaj.cena", 33.0*casa::C::arcsec);
          params2->add("shape.bmin.cena", 22.0*casa::C::arcsec);
          params2->add("shape.bpa.cena", -57*casa::C::degree);
          for (casa::uInt ant=0; ant<nAnt; ++ant) {
               params2->add("gain.g11."+toString(ant),1.1);
               params2->add("gain.g22."+toString(ant),0.9);
          }

          p2.reset(new ComponentEquation(*params2, idi));
          eq2.reset(new GainCalibrationEquation(*params2,idi,*p2));

        }

        void testSolve()
        {
          // Predict with the "perfect" parameters"
          eq1->predict();
          //std::cout<<*params1<<std::endl;
          // Calculate gradients using "imperfect" parameters"
          NormalEquations ne(*params2);
          eq2->calcEquations(ne);
          /*
          Quality q;
          LinearSolver solver1(*params2);
          solver1.addNormalEquations(ne);
          solver1.setAlgorithm("SVD");
          solver1.solveNormalEquations(q);  
          */
        }
      
    };
    
  } // namespace synthesis
} // namespace conrad

#endif // #ifndef GAIN_CALIBRATION_EQUATION_TEST_H

