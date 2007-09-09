#include <fitting/PolynomialEquation.h>
#include <fitting/CompositeEquation.h>
#include <fitting/LinearSolver.h>

#include <cppunit/extensions/HelperMacros.h>

#include <conrad/ConradError.h>
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
      CPPUNIT_TEST(testSolutionNESVD);
      CPPUNIT_TEST(testSolutionNEChol);
      CPPUNIT_TEST(testComposite);
      CPPUNIT_TEST_SUITE_END();

      private:
        PolynomialEquation *itsPolyPerfect, *itsPolyWrong;
        casa::Vector<double> itsArguments;
        casa::Vector<double> itsDataPerfect;
        casa::Vector<double> itsDataWrong;
        casa::Vector<double> itsWeights;
        casa::Vector<double> itsModelPerfect;
        casa::Vector<double> itsModelWrong;

      public:
        void setUp()
        {
        	/// The arguments are the independent variables
          itsArguments.resize(10);
          for (uint i=0;i<itsArguments.size();i++)
          {
            itsArguments[i]=i;
          }
          /// The data will be filled in
          itsDataPerfect.resize(10);
          itsDataPerfect.set(0.0);
          itsDataWrong.resize(10);
          itsDataWrong.set(0.0);
          itsWeights.resize(10);
          itsWeights.set(1.0);
          /// The model is the prediction of the data for the current parameters
          itsModelPerfect.resize(10);
          itsModelPerfect.set(0.0);
          itsModelWrong.resize(10);
          itsModelWrong.set(0.0);

          /// Set the true parameters
          Params ipPerfect;
          casa::Vector<double> quadratic(3);
          quadratic(0)=1;
          quadratic(1)=2;
          quadratic(2)=3;
          ipPerfect.add("poly", quadratic);
          
          Params ipWrong;
          casa::Vector<double> guess(3);
          guess(0)=2;
          guess(1)=-3;
          guess(2)=5;
          ipWrong.add("poly", guess);

          itsPolyPerfect = new PolynomialEquation(ipPerfect, itsDataPerfect, itsWeights, itsArguments, itsModelPerfect);
          itsPolyPerfect->predict();
          itsDataPerfect=itsModelPerfect.copy();
          delete itsPolyPerfect;
          itsPolyPerfect = new PolynomialEquation(ipPerfect, itsDataPerfect, itsWeights, itsArguments, itsModelPerfect);
          itsPolyPerfect->predict();
                    
          itsDataWrong=itsDataPerfect.copy();
          itsPolyWrong = new PolynomialEquation(ipWrong, itsDataWrong, itsWeights, itsArguments, itsModelWrong);
          itsPolyWrong->predict();
          
        }

        void tearDown()
        {
          delete itsPolyPerfect;
          delete itsPolyWrong;
        }

        void testConstructors()
        {
          CPPUNIT_ASSERT(itsPolyPerfect->parameters().names().size()==1);
          CPPUNIT_ASSERT(itsPolyPerfect->parameters().names()[0]=="poly");
        }

        void testCopy()
        {
          PolynomialEquation poly2(*itsPolyPerfect);
          CPPUNIT_ASSERT(poly2.parameters().names().size()==1);
          CPPUNIT_ASSERT(poly2.parameters().names()[0]=="poly");
        }

        void testPredict()
        {
          itsPolyPerfect->predict();
          CPPUNIT_ASSERT(itsModelPerfect[0]==1);         // 1
          CPPUNIT_ASSERT(itsModelPerfect[4]==57);        // 1+2*4+3*16=57
          CPPUNIT_ASSERT(itsModelPerfect[9]==262);       // 1+2*9+3*81=1+18+241=262
        }

        void testSolutionNESVD()
        {
          NormalEquations normeq(itsPolyWrong->parameters());
          itsPolyWrong->calcEquations(normeq);
          LinearSolver solver(itsPolyWrong->parameters());
          solver.addNormalEquations(normeq);

          Quality q;
          solver.setAlgorithm("SVD");
//          std::cout << "Before " << solver.parameters().value("poly") << std::endl;
          solver.solveNormalEquations(q);
//          std::cout << "After  " << solver.parameters().value("poly") << std::endl;
          //std::cout << q << std::endl;
          CPPUNIT_ASSERT(abs(q.cond()-11500.5)<1.0);
        }
        
        void testSolutionNEChol()
        {
          NormalEquations normeq(itsPolyWrong->parameters());
          itsPolyWrong->calcEquations(normeq);
          LinearSolver solver(itsPolyWrong->parameters());
          solver.addNormalEquations(normeq);

          Quality q;
//          std::cout << "Before " << solver.parameters().value("poly") << std::endl;
          solver.solveNormalEquations(q);
//          std::cout << "After  " << solver.parameters().value("poly") << std::endl;
//          std::cout << q << std::endl;
        }

        void testComposite()
        {
          CompositeEquation comp;
          comp.add(*itsPolyPerfect);
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
