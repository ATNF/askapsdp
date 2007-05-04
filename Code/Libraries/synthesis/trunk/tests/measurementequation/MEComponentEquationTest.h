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
	
class MEComponentEquationTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(MEComponentEquationTest);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST_EXCEPTION(testParameters, std::invalid_argument);
    CPPUNIT_TEST(testPredict);
    CPPUNIT_TEST(testDesignMatrix);
    CPPUNIT_TEST(testAssembly);
    CPPUNIT_TEST(testSVD);
    CPPUNIT_TEST(testConstructNormalEquations);
	CPPUNIT_TEST(testSolveNormalEquations);
	CPPUNIT_TEST_EXCEPTION(testNoFree, std::domain_error);
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
    
    void testCopy() 
    {
		MEParams ip;
		ip.add("Value0");
		ip.add("Value1");
		ip.add("Value2");
		delete p1;
		p1 = new MEComponentEquation(ip);
		delete p2;
		p2 = new MEComponentEquation(*p1);
		CPPUNIT_ASSERT(p2->parameters().names().size()==3);
		CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
		CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
		CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
    }
    
	void testPredict()
	{
		p1->predict(*ida);
	}
	
	void testDesignMatrix()
	{
		MEDesignMatrix dm1(*params1);
		p1->calcEquations(*ida, dm1);
		CPPUNIT_ASSERT(abs(dm1.fit()-100.0)<0.01);
		p1->predict(*ida);
		dm1.reset();
		p1->calcEquations(*ida, dm1);
		CPPUNIT_ASSERT(dm1.fit()<0.03);
		MEDesignMatrix dm2(*params2);
		p2->calcEquations(*ida, dm2);
		CPPUNIT_ASSERT(abs(dm2.fit()-7.02399)<0.0001);
	}
	
	void testAssembly() {
		// Predict with the "perfect" parameters"
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
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
		MEDesignMatrix dm1(*params1);
		p2->calcEquations(*ida, dm1);
		MENormalEquations normeq(dm1, MENormalEquations::COMPLETE);
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
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		MENormalEquations normeq(dm1, MENormalEquations::COMPLETE);
		solver1.addNormalEquations(normeq);
		solver1.solveNormalEquations(q);
	}

	void testNoFree() {
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
  	    solver1.parameters().fix("flux.i.cena");
	    solver1.parameters().fix("direction.ra.cena");
	    solver1.parameters().fix("direction.dec.cena");
		// Should throw exception: domain_error
		solver1.solveDesignMatrix(q);
	}
	
	void testParameters()
	// Should throw invalid_argument because of the lack of parameters
	{
		pempty->predict(*ida);
	}
  };
  
}
}
