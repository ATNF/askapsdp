#include <measurementequation/ImageFFTEquation.h>
#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/AntennaIllumVisGridder.h>
#include <measurementequation/ImageSolver.h>
#include <dataaccess/DataIteratorStub.h>
#include <fitting/ParamsCASATable.h>

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/BasicSL/Constants.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

#include <boost/shared_ptr.hpp>

using namespace conrad::scimath;

namespace conrad
{
  namespace synthesis
  {

    class ImageFFTEquationTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(ImageFFTEquationTest);
      CPPUNIT_TEST(testPredict);
      CPPUNIT_TEST(testSolveSphFun);
      CPPUNIT_TEST(testSolveBox);
      CPPUNIT_TEST(testSolveAntIllum);
      CPPUNIT_TEST_EXCEPTION(testFixed, std::domain_error);
      CPPUNIT_TEST_SUITE_END();

      private:
        ImageFFTEquation *p1, *p2;
        Params *params1, *params2;
        IDataSharedIter idi;

      public:
        void setUp()
        {
          idi = IDataSharedIter(new DataIteratorStub(1));

          uint npix=512;
          Axes imageAxes;
          double arcsec=casa::C::pi/(3600.0*180.0);
          double cell=5.0*arcsec;
          imageAxes.add("RA", -double(npix)*cell/2.0, double(npix)*cell/2.0);
          imageAxes.add("DEC", -double(npix)*cell/2.0, double(npix)*cell/2.0);

          params1 = new Params;
          casa::Array<double> imagePixels1(casa::IPosition(2, npix, npix));
          imagePixels1.set(0.0);
          imagePixels1(casa::IPosition(2, npix/2, npix/2))=1.0;
          imagePixels1(casa::IPosition(2, 3*npix/8, 7*npix/16))=0.7;
          params1->add("image.i.cena", imagePixels1, imageAxes);

          p1 = new ImageFFTEquation(*params1, idi);

          params2 = new Params;
          casa::Array<double> imagePixels2(casa::IPosition(2, npix, npix));
          imagePixels2.set(0.0);
          imagePixels2(casa::IPosition(2, npix/2, npix/2))=0.9;
          imagePixels2(casa::IPosition(2, 3*npix/8, 7*npix/16))=0.75;
          params2->add("image.i.cena", imagePixels2, imageAxes);

          p2 = new ImageFFTEquation(*params2, idi);

        }

        void tearDown()
        {
          delete params1;
          delete params2;
          delete p1;
          delete p2;
        }

        void testPredict()
        {
          {
            ParamsCASATable pt("ImageFFTEquationTest_original.tab", false);
            pt.setParameters(*params1);
          }
          p1->predict();
        }

        void testSolveSphFun()
        {

// Predict with the "perfect" parameters"
          NormalEquations ne(*params1);

          p1->predict();
// Calculate gradients using "imperfect" parameters"
          p2->calcEquations(ne);
          Quality q;
          ImageSolver solver1(*params2);
          solver1.addNormalEquations(ne);
          solver1.solveNormalEquations(q);
          casa::Array<double> improved=solver1.parameters().value("image.i.cena");
          uint npix=512;
          {
            ParamsCASATable pt("ImageFFTEquationTest_SphFun.tab", false);
            pt.setParameters(solver1.parameters());
          }
// This only works for the pixels with emission but it's a good test nevertheless
          CPPUNIT_ASSERT(abs(improved(casa::IPosition(2, npix/2, npix/2))-1.0)<0.003);
          CPPUNIT_ASSERT(abs(improved(casa::IPosition(2, 3*npix/8, 7*npix/16))-0.700)<0.003);
        }

        void testSolveBox()
        {
// Predict with the "perfect" parameters"
          NormalEquations ne(*params1);
          IVisGridder::ShPtr gridder=IVisGridder::ShPtr(new BoxVisGridder());
          p1 = new ImageFFTEquation(*params1, idi, gridder);
          p2 = new ImageFFTEquation(*params2, idi, gridder);

          p1->predict();
// Calculate gradients using "imperfect" parameters"
          p2->calcEquations(ne);
          Quality q;
          ImageSolver solver1(*params2);
          solver1.addNormalEquations(ne);
          solver1.solveNormalEquations(q);
          casa::Array<double> improved=solver1.parameters().value("image.i.cena");
          uint npix=512;
          {
            ParamsCASATable pt("ImageFFTEquationTest_Box.tab", false);
            pt.setParameters(solver1.parameters());
          }
// This only works for the pixels with emission but it's a good test nevertheless
          CPPUNIT_ASSERT(abs(improved(casa::IPosition(2, npix/2, npix/2))-1.0)<0.003);
          CPPUNIT_ASSERT(abs(improved(casa::IPosition(2, 3*npix/8, 7*npix/16))-0.700)<0.003);
        }

        void testSolveAntIllum()
        {
// Predict with the "perfect" parameters"
          NormalEquations ne(*params1);
          IVisGridder::ShPtr gridder=IVisGridder::ShPtr(new AntennaIllumVisGridder(12.0, 1.0));
          p1 = new ImageFFTEquation(*params1, idi, gridder);
          p2 = new ImageFFTEquation(*params2, idi, gridder);

          p1->predict();
// Calculate gradients using "imperfect" parameters"
          p2->calcEquations(ne);
          Quality q;
          ImageSolver solver1(*params2);
          solver1.addNormalEquations(ne);
          solver1.solveNormalEquations(q);
          casa::Array<double> improved=solver1.parameters().value("image.i.cena");
          uint npix=512;
          {
            ParamsCASATable pt("ImageFFTEquationTest_AntIllum.tab", false);
            pt.setParameters(solver1.parameters());
          }
// This only works for the pixels with emission but it's a good test nevertheless
          CPPUNIT_ASSERT(abs(improved(casa::IPosition(2, npix/2, npix/2))-1.0)<0.003);
          CPPUNIT_ASSERT(abs(improved(casa::IPosition(2, 3*npix/8, 7*npix/16))-0.700)<0.005);
        }

        void testFixed()
        {
          NormalEquations ne(*params1);
          p1->predict();
          p2->calcEquations(ne);
          Quality q;
          LinearSolver solver1(*params2);
          solver1.addNormalEquations(ne);
// Should throw exception: domain_error
          solver1.parameters().fix("image.i.cena");
          solver1.solveNormalEquations(q);
        }
    };

  }
}
