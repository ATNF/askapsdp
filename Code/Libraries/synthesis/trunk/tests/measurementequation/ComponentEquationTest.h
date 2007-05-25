#include <measurementequation/ComponentEquation.h>
#include <fitting/LinearSolver.h>
#include <dataaccess/DataIteratorStub.h>
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

#include <boost/shared_ptr.hpp>

using namespace conrad::scimath;

namespace conrad {
namespace synthesis {
	
class ComponentEquationTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(ComponentEquationTest);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testPredict);
    CPPUNIT_TEST(testAssembly);
    CPPUNIT_TEST(testConstructNormalEquations);
	CPPUNIT_TEST(testSolveNormalEquations);
	CPPUNIT_TEST_EXCEPTION(testNoFree, std::domain_error);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    ComponentEquation *p1, *p2;
	Params *params1, *params2, *params3;
    IDataSharedIter idi;

  public:
    void setUp()
    {
      idi= IDataSharedIter(new DataIteratorStub(1));
      
	  params1 = new Params;
	  params1->add("flux.i.cena", 100.0);
	  params1->add("direction.ra.cena", 0.5);
	  params1->add("direction.dec.cena", -0.3);

      p1 = new ComponentEquation(*params1, idi);

	  params2 = new Params;
	  params2->add("flux.i.cena", 100.0);
	  params2->add("direction.ra.cena", 0.500005);
	  params2->add("direction.dec.cena", -0.300003);
	  	  
      p2 = new ComponentEquation(*params2, idi);
      
    }
    
    void tearDown() 
    {
      delete p1;
      delete p2;
    }
    
    void testCopy() 
    {
		Params ip;
		ip.add("Value0");
		ip.add("Value1");
		ip.add("Value2");
		delete p1;
		p1 = new ComponentEquation(ip, idi);
		delete p2;
		p2 = new ComponentEquation(*p1);
		CPPUNIT_ASSERT(p2->parameters().names().size()==3);
		CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
		CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
		CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
    }
    
	void testPredict()
	{
		p1->predict();
	}
	
	void testAssembly() {
		// Predict with the "perfect" parameters"
		NormalEquations ne(*params1);
		p1->predict();
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(ne);
		LinearSolver solver1(*params2);
		solver1.addNormalEquations(ne);
	}

	void testConstructNormalEquations() {
		NormalEquations ne(*params1);
		p2->calcEquations(ne);
		std::map<string, std::map<string, casa::Matrix<double> > > nm(ne.normalMatrix());
		vector<string> names(params1->names());
		for (uint row=0;row<names.size();row++) {
			for (uint col=0;col<names.size();col++) {
				casa::IPosition ip(nm[names[row]][names[col]].shape());
				CPPUNIT_ASSERT(ip(0)==1);
				CPPUNIT_ASSERT(ip(1)==1);
			}
		}
	}

	void testSolveNormalEquations() {
		// Predict with the "perfect" parameters"
		p1->predict();
		// Calculate gradients using "imperfect" parameters" 
		NormalEquations ne(*params1);
		p2->calcEquations(ne);
		Quality q;
		LinearSolver solver1(*params2);
		solver1.addNormalEquations(ne);
		solver1.solveNormalEquations(q, true);
        CPPUNIT_ASSERT(abs(q.cond()-3.78547e+12)<1e7);
	}

	void testNoFree() {
		NormalEquations ne(*params1);
		p1->predict();
		p2->calcEquations(ne);
		Quality q;
		LinearSolver solver1(*params2);
		solver1.addNormalEquations(ne);
  	    solver1.parameters().fix("flux.i.cena");
	    solver1.parameters().fix("direction.ra.cena");
	    solver1.parameters().fix("direction.dec.cena");
		// Should throw exception: domain_error
		solver1.solveNormalEquations(q);
    }
	
  };
  
}
}
