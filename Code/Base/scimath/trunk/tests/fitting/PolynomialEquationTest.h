#include <fitting/PolynomialEquation.h>
#include <fitting/CompositeEquation.h>
#include <fitting/LinearSolver.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>
#include <cmath>

using std::abs;

namespace conrad
{
  namespace scimath
  {

    class PolynomialEquationTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(PolynomialEquationTest);
      CPPUNIT_TEST(testConstructors);
      CPPUNIT_TEST(testCopy);
      CPPUNIT_TEST(testPredict);
      CPPUNIT_TEST(testSolutionNE);
      CPPUNIT_TEST(testComposite);
      CPPUNIT_TEST_SUITE_END();

      private:
        PolynomialEquation *itsPoly1, *itsPoly2;
        casa::Vector<double> itsArguments;
        casa::Vector<double> itsData;
        casa::Vector<double> itsWeights;
        casa::Vector<double> itsModel;

      public:
        void setUp()
        {
          itsArguments.resize(10);
          for (uint i=0;i<itsArguments.size();i++)
          {
            itsArguments[i]=i;
          }
          itsData.resize(10);
          itsData.set(0.0);
          itsWeights.resize(10);
          itsWeights.set(1.0);
          itsModel.resize(10);
          itsModel.set(0.0);
          Params ip;
          casa::Vector<double> quadratic(3);
          quadratic(0)=1;
          quadratic(1)=2;
          quadratic(2)=3;
          ip.add("poly", quadratic);
          itsPoly1 = new PolynomialEquation(ip, itsData, itsWeights, itsArguments, itsModel);
        }

        void tearDown()
        {
          delete itsPoly1;
          delete itsPoly2;
        }

        void testConstructors()
        {
          CPPUNIT_ASSERT(itsPoly1->parameters().names().size()==1);
          CPPUNIT_ASSERT(itsPoly1->parameters().names()[0]=="poly");
        }

        void testCopy()
        {
          delete itsPoly2;
          itsPoly2 = new PolynomialEquation(*itsPoly1);
          CPPUNIT_ASSERT(itsPoly2->parameters().names().size()==1);
          CPPUNIT_ASSERT(itsPoly2->parameters().names().size()==1);
          CPPUNIT_ASSERT(itsPoly2->parameters().names()[0]=="poly");
        }

        void testPredict()
        {
          itsPoly1->predict();
          CPPUNIT_ASSERT(itsModel[0]==1);         // 1
          CPPUNIT_ASSERT(itsModel[4]==57);        // 1+2*4+3*16=57
          CPPUNIT_ASSERT(itsModel[9]==262);       // 1+2*9+3*81=1+18+241=262
        }

        void testSolutionNE()
        {
          itsPoly1->predict();
          Params ip(itsPoly1->parameters());
          NormalEquations normeq(ip);
          itsPoly1->calcEquations(normeq);
          casa::Vector<double> pvals(ip.value("poly").size());
          pvals.set(0.0);
          ip.update("poly", pvals);
          LinearSolver solver(ip);
          solver.addNormalEquations(normeq);
          Quality q;
          solver.setAlgorithm("SVD");
          solver.solveNormalEquations(q);
          CPPUNIT_ASSERT(abs(q.cond()-11500.5)<1.0);
        }

        void testComposite()
        {
          CompositeEquation comp(itsPoly1->parameters());
          comp.add(*itsPoly1);
          comp.predict();
          Params ip(comp.parameters());
          NormalEquations normeq(ip);
          comp.calcEquations(normeq);
          casa::Vector<double> pvals(ip.value("poly").size());
          pvals.set(0.0);
          ip.update("poly", pvals);
          LinearSolver solver(ip);
          solver.addNormalEquations(normeq);
          Quality q;
          solver.setAlgorithm("SVD");
          solver.solveNormalEquations(q);
          CPPUNIT_ASSERT(abs(q.cond()-11500.5)<1.0);
        }

    };

  }
}
