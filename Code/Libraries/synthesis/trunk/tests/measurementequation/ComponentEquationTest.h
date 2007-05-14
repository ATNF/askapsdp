#include <measurementequation/ComponentEquation.h>
#include <fitting/LinearSolver.h>
#include <dataaccess/DataAccessorStub.h>
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
    CPPUNIT_TEST(testDesignMatrix);
    CPPUNIT_TEST(testAssembly);
    CPPUNIT_TEST(testSVD);
    CPPUNIT_TEST(testConstructNormalEquations);
	CPPUNIT_TEST(testSolveNormalEquations);
	CPPUNIT_TEST_EXCEPTION(testNoFree, std::domain_error);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    ComponentEquation *p1, *p2;
	Params *params1, *params2, *params3;
    boost::shared_ptr<IDataAccessor> ida;

  public:
    void setUp()
    {
      ida = boost::shared_ptr<IDataAccessor>(new DataAccessorStub(true));
      
	  params1 = new Params;
	  params1->add("flux.i.cena", 100.0);
	  params1->add("direction.ra.cena", 0.5);
	  params1->add("direction.dec.cena", -0.3);

      p1 = new ComponentEquation(*params1, ida);

	  params2 = new Params;
	  params2->add("flux.i.cena", 100.0);
	  params2->add("direction.ra.cena", 0.500005);
	  params2->add("direction.dec.cena", -0.300003);
	  	  
      p2 = new ComponentEquation(*params2, ida);
      
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
		p1 = new ComponentEquation(ip, ida);
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
	
	void testDesignMatrix()
	{
		DesignMatrix dm1(*params1);
		p1->calcEquations(dm1);
		CPPUNIT_ASSERT(abs(dm1.fit()-100.0/sqrt(2.0))<0.01);
		p1->predict();
		dm1.reset();
		p1->calcEquations(dm1);
        CPPUNIT_ASSERT(dm1.fit()<0.021);
		DesignMatrix dm2(*params2);
		p2->calcEquations(dm2);
		CPPUNIT_ASSERT(abs(dm2.fit()-4.96671)<0.0001);
	}
	
	void testAssembly() {
		// Predict with the "perfect" parameters"
		DesignMatrix dm1(*params1);
		p1->predict();
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(dm1);
		Quality q;
		LinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
	}

	void testSVD() {
		// Predict with the "perfect" parameters"
		p1->predict();
		// Calculate gradients using "imperfect" parameters" 
		DesignMatrix dm1(*params1);
		p2->calcEquations(dm1);
		Quality q;
		LinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==3);
        CPPUNIT_ASSERT(abs(q.cond()-1.94563e+06)<100.0);
  	    solver1.parameters().fix("flux.i.cena");
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==2);
		CPPUNIT_ASSERT(abs(q.cond()-2.58063)<0.0001);
	    solver1.parameters().fix("direction.ra.cena");
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==1);
		CPPUNIT_ASSERT(abs(q.cond()-1.000000)<0.0001);
	}

	void testConstructNormalEquations() {
		DesignMatrix dm1(*params1);
		p2->calcEquations(dm1);
		NormalEquations normeq(dm1, NormalEquations::COMPLETE);
		std::map<string, std::map<string, casa::Matrix<double> > > nm(normeq.normalMatrix());
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
		DesignMatrix dm1(*params1);
		p2->calcEquations(dm1);
		Quality q;
		LinearSolver solver1(*params2);
		NormalEquations normeq(dm1, NormalEquations::COMPLETE);
		solver1.addNormalEquations(normeq);
		solver1.solveNormalEquations(q, true);
        CPPUNIT_ASSERT(abs(q.cond()-3.78547e+12)<1e7);
	}

	void testNoFree() {
		DesignMatrix dm1(*params1);
		p1->predict();
		p2->calcEquations(dm1);
		Quality q;
		LinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
  	    solver1.parameters().fix("flux.i.cena");
	    solver1.parameters().fix("direction.ra.cena");
	    solver1.parameters().fix("direction.dec.cena");
		// Should throw exception: domain_error
		solver1.solveDesignMatrix(q);
    }
	
  };
  
}
}
