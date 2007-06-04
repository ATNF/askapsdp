#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
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

namespace conrad {
namespace synthesis {
	
class TableVisGridderTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TableVisGridderTest);
    CPPUNIT_TEST(testForward);
    CPPUNIT_TEST(testReverse);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    BoxVisGridder *itsBox;
    SphFuncVisGridder *itsSphFunc;
    
    IDataSharedIter idi;
    Axes* itsAxes;
	casa::Cube<casa::Complex>* itsGrid;
	casa::Vector<float>* itsWeights;

  public:
    void setUp()
    {
      idi = IDataSharedIter(new DataIteratorStub(1));
            
	  Params ip;
	  ip.add("flux.i.cena", 100.0);
	  ip.add("direction.ra.cena", 0.5);
	  ip.add("direction.dec.cena", -0.3);
	  
	  ComponentEquation ce(ip, idi);
	  ce.predict();

      itsBox = new BoxVisGridder();
      itsSphFunc = new SphFuncVisGridder();

      double cellSize=10*casa::C::arcsec;

      itsAxes=new Axes();
      itsAxes->add("RA", 256*cellSize, -256*cellSize);
      itsAxes->add("DEC", -256*cellSize, 256*cellSize);

      itsGrid=new casa::Cube<casa::Complex>(512,512,1);
      itsGrid->set(0.0);
      
      itsWeights=new casa::Vector<float>(1);
      itsWeights->set(0.0);
      
    }
    
    void tearDown() 
    {
      delete itsBox;
      delete itsSphFunc;
      delete itsGrid;
      delete itsWeights;
      delete itsAxes;
    }

	void testReverse()
	{
        itsBox->reverse(idi, *itsAxes, *itsGrid, *itsWeights);
        itsSphFunc->reverse(idi, *itsAxes, *itsGrid, *itsWeights);
	}    
	void testForward()
	{
        itsBox->forward(idi, *itsAxes, *itsGrid);
        itsSphFunc->forward(idi, *itsAxes, *itsGrid);
	}    
  };
  
}
}
