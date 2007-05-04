#include <measurementequation/MEComponentEquation.h>
#include <measurementequation/MELinearSolver.h>
#include <dataaccess/DataAccessorStub.h>
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

namespace conrad {
namespace synthesis {
	
class MELinearSolverTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(MELinearSolverTest);
    CPPUNIT_TEST(testSVD);
    CPPUNIT_TEST_EXCEPTION(testFixed, std::domain_error);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    MEComponentEquation *p1, *p2, *p3, *pempty;
	MEParams *params1, *params2, *params3;
    DataAccessorStub *ida;

  public:
    void setUp()
    {
      ida = new DataAccessorStub(true);
 	  params1 = new MEParams;
	  params1->add("flux.i.cena", 100.0);
	  params1->add("direction.ra.cena", 0.5);
	  params1->add("direction.dec.cena", -0.3);

      p1 = new MEComponentEquation(*params1);

	  params2 = new MEParams;
	  params2->add("flux.i.cena", 100.0);
	  params2->add("direction.ra.cena", 0.500005);
	  params2->add("direction.dec.cena", -0.300003);
	  	  
      p2 = new MEComponentEquation(*params2);
      
      p3 = new MEComponentEquation();
      pempty = new MEComponentEquation();
    }
    
    void tearDown() 
    {
      delete ida;
      delete p1;
      delete p2;
      delete p3;
      delete pempty;
    }
	
	void testSVD() {
		// Predict with the "perfect" parameters"
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==3);
		CPPUNIT_ASSERT(abs(q.cond()-1.97889e+06)<100.0);
	    solver1.parameters().fix("direction.dec.cena");
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==2);
		CPPUNIT_ASSERT(abs(q.cond()-1.79105e+06)<100.0);
	    solver1.parameters().fix("direction.ra.cena");
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==1);
		CPPUNIT_ASSERT(abs(q.cond()-1.000000)<0.0001);
	}
	void testFixed() {
		// Predict with the "perfect" parameters"
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
	    solver1.parameters().fix("direction.dec.cena");
	    solver1.parameters().fix("direction.ra.cena");
  	    solver1.parameters().fix("flux.i.cena");
		// Should throw exception: domain_error since all parameters
		// are fixed
		solver1.solveDesignMatrix(q);
	}
	
  };
  
}
}
