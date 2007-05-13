#include <fitting/PolynomialEquation.h>
#include <fitting/LinearSolver.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

namespace conrad {
namespace scimath {
	
class PolynomialEquationTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(PolynomialEquationTest);
    CPPUNIT_TEST(testConstructors);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testPredict);
    CPPUNIT_TEST(testDesignMatrix);
    CPPUNIT_TEST(testSolutionDM);
    CPPUNIT_TEST(testSolutionNE);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    PolynomialEquation *p1, *p2, *p3, *pempty;
    
  public:
    void setUp()
    {
      Params ip;
      casa::Vector<double> quadratic(3);
      quadratic(0)=1;
      quadratic(1)=2;
      quadratic(2)=3;
      ip.add("poly", quadratic);
      p1 = new PolynomialEquation(ip);
      p2 = new PolynomialEquation();
      p3 = new PolynomialEquation();
      pempty = new PolynomialEquation();

    }
        
    void tearDown() 
    {
      delete p1;
      delete p2;
      delete p3;
      delete pempty;
    }
    
    void testConstructors()
    {
        CPPUNIT_ASSERT(p1->parameters().names().size()==1);
        CPPUNIT_ASSERT(p1->parameters().names()[0]=="poly");
    }
    
    void testCopy() 
    {
		delete p2;
		p2 = new PolynomialEquation(*p1);
        CPPUNIT_ASSERT(p2->parameters().names().size()==1);
        CPPUNIT_ASSERT(p2->parameters().names()[0]=="poly");
    }
    
    void testPredict()
    {
        casa::Vector<double> x(10);
        for (uint i=0;i<x.size();i++) {
            x[i]=i;
        }
        casa::Vector<double> values(10);
        p1->predict(x, values);
        CPPUNIT_ASSERT(values[0]==1); // 1 
        CPPUNIT_ASSERT(values[4]==57); // 1+2*4+3*16=57
        CPPUNIT_ASSERT(values[9]==262); // 1+2*9+3*81=1+18+241=262
    }  
    
    void testDesignMatrix()
    {
        casa::Vector<double> x(10);
        for (uint i=0;i<x.size();i++) {
            x[i]=i;
        }
        casa::Vector<double> values(10);
        DesignMatrix dm(p1->parameters());
        p1->predict(x, values);
        p1->calcEquations(values, x, dm);
    }  
    void testSolutionDM()
    {
        casa::Vector<double> x(10);
        for (uint i=0;i<x.size();i++) {
            x[i]=i;
        }
        casa::Vector<double> values(10);
        DesignMatrix dm(p1->parameters());
        p1->predict(x, values);
        p1->calcEquations(values, x, dm);
        Params ip(p1->parameters());
        casa::Vector<double> pvals(ip.value("poly").size());
        pvals.set(0.0);
        ip.update("poly", pvals);
        LinearSolver solver(ip);
        solver.addDesignMatrix(dm);
        Quality q;
        solver.solveDesignMatrix(q);
        CPPUNIT_ASSERT(abs(q.cond()-107.24)<0.1);
    }  
    void testSolutionNE()
    {
        casa::Vector<double> x(10);
        for (uint i=0;i<x.size();i++) {
            x[i]=i;
        }
        casa::Vector<double> values(10);
        DesignMatrix dm(p1->parameters());
        p1->predict(x, values);
        p1->calcEquations(values, x, dm);
        NormalEquations normeq(dm, NormalEquations::COMPLETE);
        Params ip(p1->parameters());
        casa::Vector<double> pvals(ip.value("poly").size());
        pvals.set(0.0);
        ip.update("poly", pvals);
        LinearSolver solver(ip);
        solver.addNormalEquations(normeq);
        Quality q;
        solver.solveNormalEquations(q, true);
        CPPUNIT_ASSERT(abs(q.cond()-11500.5)<1.0);
    }  
    
  };
  
}
}
