#include <measurementequation/ImageFFTEquation.h>
#include <fitting/LinearSolver.h>
#include <dataaccess/DataIteratorStub.h>
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

namespace conrad {
namespace synthesis {
	
class ImageFFTEquationTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(ImageFFTEquationTest);
    CPPUNIT_TEST(testPredict);
    CPPUNIT_TEST(testSVD);
    CPPUNIT_TEST_EXCEPTION(testFixed, std::domain_error);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    ImageFFTEquation *p1, *p2;
	Params *params1, *params2, *params3;
    IDataSharedIter idi;

  public:
    void setUp()
    {
      idi = IDataSharedIter(new DataIteratorStub(1));
      
	  uint npix=16;
      Domain imageDomain;
	  double arcsec=casa::C::pi/(3600.0*180.0);
      imageDomain.add("RA", -60.0*arcsec, +60.0*arcsec, npix); 
      imageDomain.add("DEC", -600.0*arcsec, +60.0*arcsec, npix); 

	  params1 = new Params;
	  casa::Vector<double> imagePixels1(npix*npix);
	  imagePixels1.set(0.0);
	  imagePixels1(npix/2+npix*npix/2)=1.0;
	  imagePixels1(10+npix*5)=0.7;
	  params1->add("image.i.cena", imagePixels1, imageDomain);

      p1 = new ImageFFTEquation(*params1, idi);

	  params2 = new Params;
	  casa::Vector<double> imagePixels2(npix*npix);
	  imagePixels2.set(0.0);
	  imagePixels2(npix/2+npix*npix/2)=0.9;
	  imagePixels2(10+npix*5)=0.75;
	  params2->add("image.i.cena", imagePixels2, imageDomain);
	  	  
      p2 = new ImageFFTEquation(*params2, idi);
      
    }
    
    void tearDown() 
    {
      delete p1;
      delete p2;
  	}
        
	void testPredict()
	{
		p1->predict();
	}
	
	void testSVD() {
		// Predict with the "perfect" parameters"
		NormalEquations ne(*params1);
		p1->predict();
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(ne);
		Quality q;
		LinearSolver solver1(*params2);
		solver1.addNormalEquations(ne);
		solver1.solveNormalEquations(q, true);
        CPPUNIT_ASSERT(abs(q.cond()-1.77101e+14)<1e9);
		casa::Vector<double> improved=solver1.parameters().value("image.i.cena");
		uint npix=16;
		CPPUNIT_ASSERT(abs(improved(npix/2+npix*npix/2)-1.0)<0.003);
		CPPUNIT_ASSERT(abs(improved(10+npix*5)-0.700)<0.003);
	}
	
	void testFixed() {
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
