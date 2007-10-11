#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
//#include <gridding/AntennaIllumVisGridder.h>
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
        //AntennaIllumVisGridder *itsAnt;
        WProjectVisGridder *itsWProject;

        IDataSharedIter idi;
        Axes* itsAxes;
        casa::Array<double>* itsModel;
        casa::Array<double>* itsModelPSF;
        casa::Array<double>* itsModelWeights;

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
          //itsAnt = new AntennaIllumVisGridder(12.0, 1.0, 10000.0, 8, 1e-3, 1, 128, 1);
          itsWProject = new WProjectVisGridder(10000.0, 8, 1e-3, 1, 128, "");

          double cellSize=10*casa::C::arcsec;

          itsAxes=new Axes();
          itsAxes->add("RA", 256*cellSize, -256*cellSize);
          itsAxes->add("DEC", -256*cellSize, 256*cellSize);

          itsModel=new casa::Array<double>(casa::IPosition(4,512,512,1,1));
          itsModel->set(0.0);
          itsModelPSF=new casa::Array<double>(casa::IPosition(4,512,512,1,1));
          itsModelPSF->set(0.0);
          itsModelWeights=new casa::Array<double>(casa::IPosition(4,512,512,1,1));
          itsModelWeights->set(0.0);
        }

        void tearDown()
        {
          delete itsBox;
          delete itsSphFunc;
          //delete itsAnt;
          delete itsModel;
          delete itsModelPSF;
          delete itsModelWeights;
          delete itsAxes;
        }

        void testReverseBox()
        {
          itsBox->initialiseGrid(*itsAxes, itsModel->shape(), true);
          itsBox->grid(idi);
          itsBox->finaliseGrid(*itsModel);
          itsBox->finalisePSF(*itsModelPSF);
          itsBox->finaliseWeights(*itsModelWeights);
        }
        void testForwardBox()
        {
        	itsBox->initialiseDegrid(*itsAxes, *itsModel);
          itsBox->degrid(idi);
        }
        void testReverseSph()
        {
          itsSphFunc->initialiseGrid(*itsAxes, itsModel->shape(), true);
          itsSphFunc->grid(idi);
          itsSphFunc->finaliseGrid(*itsModel);
          itsSphFunc->finalisePSF(*itsModelPSF);
          itsSphFunc->finaliseWeights(*itsModelWeights);
        }
        void testForwardSph()
        {
        	itsSphFunc->initialiseDegrid(*itsAxes, *itsModel);
          itsSphFunc->degrid(idi);
        }
        void testReverseAnt()
        {
	  /*
          itsAnt->initialiseGrid(*itsAxes, itsModel->shape(), true);
          itsAnt->grid(idi);
          itsAnt->finaliseGrid(*itsModel);
          itsAnt->finalisePSF(*itsModelPSF);
          itsAnt->finaliseWeights(*itsModelWeights);
	  */
        }
        void testForwardAnt()
        {
		/*
        	itsAnt->initialiseDegrid(*itsAxes, *itsModel);
          itsAnt->degrid(idi);
	  */
        }
        void testReverseWProject()
        {
          itsWProject->initialiseGrid(*itsAxes, itsModel->shape(), true);
          itsWProject->grid(idi);
          itsWProject->finaliseGrid(*itsModel);
          itsWProject->finalisePSF(*itsModelPSF);
          itsWProject->finaliseWeights(*itsModelWeights);
        }
        void testForwardWProject()
        {
        	itsWProject->initialiseDegrid(*itsAxes, *itsModel);
          itsWProject->degrid(idi);
        }
    };

  }
}
