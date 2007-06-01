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
	casa::Vector<double>* cellSize;
	casa::Cube<casa::Complex>* grid;
	casa::Vector<float>* weights;

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


      cellSize=new casa::Vector<double>(2);

      (*cellSize)(0)=1.0/(10.0*casa::C::arcsec);
      (*cellSize)(1)=1.0/(10.0*casa::C::arcsec);
      
      grid=new casa::Cube<casa::Complex>(512,512,1);
      grid->set(0.0);
      
      weights=new casa::Vector<float>(1);
      weights->set(0.0);
      
    }
    
    void tearDown() 
    {
      delete itsBox;
      delete itsSphFunc;
      delete cellSize;
      delete grid;
      delete weights;
    }

	void testReverse()
	{
        itsBox->reverse(idi, *cellSize, *grid, *weights);
        itsSphFunc->reverse(idi, *cellSize, *grid, *weights);
	}    
	void testForward()
	{
        itsBox->forward(idi, *cellSize, *grid);
        itsSphFunc->forward(idi, *cellSize, *grid);
	}    
  };
  
}
}
