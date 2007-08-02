#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/AntennaIllumVisGridder.h>
#include <gridding/WProjectVisGridder.h>
#include <fitting/Params.h>
#include <measurementequation/ComponentEquation.h>
#include <dataaccess/DataIteratorStub.h>
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/BasicSL/Constants.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

using namespace conrad::scimath;

namespace conrad
{
  namespace synthesis
  {

    class TableVisGridderTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(TableVisGridderTest);
      CPPUNIT_TEST(testForwardBox);
      CPPUNIT_TEST(testReverseBox);
      CPPUNIT_TEST(testForwardSph);
      CPPUNIT_TEST(testReverseSph);
      CPPUNIT_TEST(testForwardAnt);
      CPPUNIT_TEST(testReverseAnt);
      CPPUNIT_TEST(testForwardWProject);
      CPPUNIT_TEST(testReverseWProject);
      CPPUNIT_TEST_SUITE_END();

      private:
        BoxVisGridder *itsBox;
        SphFuncVisGridder *itsSphFunc;
        AntennaIllumVisGridder *itsAnt;
        WProjectVisGridder *itsWProject;

        IDataSharedIter idi;
        Axes* itsAxes;
        casa::Cube<casa::Complex>* itsGrid;
        casa::Matrix<double>* itsWeights;

      public:
        void setUp()
        {
          idi = IDataSharedIter(new DataIteratorStub(1));

          Params ip;
          ip.add("flux.i.cena", 100.0);
          ip.add("direction.ra.cena", 0.5);
          ip.add("direction.dec.cena", -0.3);
          ip.add("shape.bmaj.cena", 0.0);
          ip.add("shape.bmin.cena", 0.0);
          ip.add("shape.bpa.cena", 0.0);

          ComponentEquation ce(ip, idi);
          ce.predict();

          itsBox = new BoxVisGridder();
          itsSphFunc = new SphFuncVisGridder();
          itsAnt = new AntennaIllumVisGridder(12.0, 1.0, 10000.0, 8, 1e-3, 1, 128, 1);
          itsWProject = new WProjectVisGridder(10000.0, 8, 1e-3, 1, 128);

          double cellSize=10*casa::C::arcsec;

          itsAxes=new Axes();
          itsAxes->add("RA", 256*cellSize, -256*cellSize);
          itsAxes->add("DEC", -256*cellSize, 256*cellSize);

          itsGrid=new casa::Cube<casa::Complex>(512,512,1);
          itsGrid->set(0.0);

          itsWeights=new casa::Matrix<double>(0,0);
          itsWeights->set(0.0);

        }

        void tearDown()
        {
          delete itsBox;
          delete itsSphFunc;
          delete itsAnt;
          delete itsGrid;
          delete itsWeights;
          delete itsAxes;
        }

        void testReverseBox()
        {
          itsBox->reverse(idi, *itsAxes, *itsGrid);
          itsBox->reverseWeights(idi, *itsWeights);
        }
        void testForwardBox()
        {
          itsBox->forward(idi, *itsAxes, *itsGrid);
        }
        void testReverseSph()
        {
          itsSphFunc->reverse(idi, *itsAxes, *itsGrid);
          itsSphFunc->reverseWeights(idi, *itsWeights);
        }
        void testForwardSph()
        {
          itsSphFunc->forward(idi, *itsAxes, *itsGrid);
        }
        void testReverseAnt()
        {
          itsAnt->reverse(idi, *itsAxes, *itsGrid);
          itsAnt->reverseWeights(idi, *itsWeights);
        }
        void testForwardAnt()
        {
          itsAnt->forward(idi, *itsAxes, *itsGrid);
        }
        void testReverseWProject()
        {
          itsWProject->reverse(idi, *itsAxes, *itsGrid);
          itsWProject->reverseWeights(idi, *itsWeights);
        }
        void testForwardWProject()
        {
          itsWProject->forward(idi, *itsAxes, *itsGrid);
        }
    };

  }
}
