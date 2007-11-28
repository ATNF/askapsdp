#include <measurementequation/ImageFFTEquation.h>
//#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/AWProjectVisGridder.h>
#include <measurementequation/ImageSolver.h>
#include <dataaccess/DataIteratorStub.h>
#include <fitting/ParamsCasaTable.h>

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/BasicSL/Constants.h>

#include <conrad/ConradError.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

#include <boost/shared_ptr.hpp>

using namespace conrad;
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
      CPPUNIT_TEST(testSolveAntIllum);
      CPPUNIT_TEST_EXCEPTION(testFixed, CheckError);
      CPPUNIT_TEST_SUITE_END();

  private:
      boost::shared_ptr<ImageFFTEquation> p1, p2;
      boost::shared_ptr<Params> params1, params2;
      IDataSharedIter idi;
      uint npix;

  public:
      void setUp()
      {
        idi = IDataSharedIter(new DataIteratorStub(1));

        npix=1024;
        Axes imageAxes;
        double arcsec=casa::C::pi/(3600.0*180.0);
        double cell=8.0*arcsec;
        imageAxes.add("RA", -double(npix)*cell/2.0, double(npix)*cell/2.0);
        imageAxes.add("DEC", -double(npix)*cell/2.0, double(npix)*cell/2.0);

        params1.reset(new Params);
        casa::Array<double> imagePixels1(casa::IPosition(4, npix, npix, 1, 1));
        imagePixels1.set(0.0);
        imagePixels1(casa::IPosition(4, npix/2, npix/2, 0, 0))=1.0;
        imagePixels1(casa::IPosition(4, 3*npix/8, 7*npix/16, 0, 0))=0.7;
        params1->add("image.i.cena", imagePixels1, imageAxes);

        p1.reset(new ImageFFTEquation(*params1, idi));

        params2.reset(new Params);
        casa::Array<double> imagePixels2(casa::IPosition(4, npix, npix, 1, 1));
        imagePixels2.set(0.0);
        imagePixels2(casa::IPosition(4, npix/2, npix/2, 0, 0))=0.9;
        imagePixels2(casa::IPosition(4, 3*npix/8, 7*npix/16, 0, 0))=0.75;
        params2->add("image.i.cena", imagePixels2, imageAxes);
        p2.reset(new ImageFFTEquation(*params2, idi));

      }

      void tearDown()
      {
      }

      void testPredict()
      {
        //        {
        //          ParamsCasaTable pt("ImageFFTEquationTest_original.tab", false);
        //          pt.setParameters(*params1);
        //        }
        p1->predict();
      }

      void testSolveSphFun()
      {
        // Predict with the "perfect" parameters"
        p1->predict();
        // Calculate gradients using "imperfect" parameters"
        ImagingNormalEquations ne(*params1);
        p2->calcEquations(ne);
        Quality q;
        ImageSolver solver1(*params1);
        solver1.addNormalEquations(ne);
        solver1.solveNormalEquations(q);
        const casa::Array<double> improved=solver1.parameters().value("image.i.cena");
        //        {
        //          ParamsCasaTable pt("ImageFFTEquationTest_SphFun.tab", false);
        //          pt.setParameters(solver1.parameters());
        //        }
        // This only works for the pixels with emission but it's a good test nevertheless
        CPPUNIT_ASSERT(abs(improved(casa::IPosition(4, npix/2, npix/2, 0, 0))
            -1.0)<0.003);
        CPPUNIT_ASSERT(abs(improved(casa::IPosition(4, 3*npix/8, 7*npix/16, 0,
            0))-0.700)<0.003);
      }

      void testSolveAntIllum()
      {
        // Predict with the "perfect" parameters"
        ImagingNormalEquations ne(*params1);
        IVisGridder::ShPtr gridder=IVisGridder::ShPtr(new AWProjectVisGridder(12.0,
            1.0, 8000, 9, 1e-3, 8, 256));
        p1.reset(new ImageFFTEquation(*params1, idi, gridder));
        p2.reset(new ImageFFTEquation(*params2, idi, gridder));
        p1->predict();
        // Calculate gradients using "imperfect" parameters"
        p2->calcEquations(ne);
        Quality q;
        ImageSolver solver1(*params1);
        solver1.addNormalEquations(ne);
        solver1.solveNormalEquations(q);
        const casa::Array<double> improved=solver1.parameters().value("image.i.cena");
        //        {
        //          ParamsCasaTable pt("ImageFFTEquationTest_AntIllum.tab", false);
        //          pt.setParameters(solver1.parameters());
        //        }
        // This only works for the pixels with emission but it's a good test nevertheless
        CPPUNIT_ASSERT(abs(improved(casa::IPosition(4, npix/2, npix/2, 0, 0))
            -1.0)<0.005);
        CPPUNIT_ASSERT(abs(improved(casa::IPosition(4, 3*npix/8, 7*npix/16, 0,
            0))-0.700)<0.005);
      }

      void testFixed()
      {
        ImagingNormalEquations ne(*params1);
        p1->predict();
        p2->calcEquations(ne);
        Quality q;
        params2->fix("image.i.cena");
        ImageSolver solver1(*params2);
        solver1.addNormalEquations(ne);
        solver1.solveNormalEquations(q);
      }
    };

  }
}
