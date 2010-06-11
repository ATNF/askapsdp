/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///

#include <fitting/PolynomialEquation.h>
#include <fitting/CompositeEquation.h>
#include <fitting/GenericNormalEquations.h>
#include <fitting/LinearSolver.h>

#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapError.h>
#include <cmath>

using std::abs;

namespace askap
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
        boost::shared_ptr<PolynomialEquation> itsPolyPerfect, itsPolyWrong;
        casa::Vector<double> itsArguments;
        casa::Vector<double> itsDataPerfect;
        casa::Vector<double> itsDataWrong;
        casa::Vector<double> itsWeights;
        casa::Vector<double> itsModelPerfect;
        casa::Vector<double> itsModelWrong;
        Params itsIPWrong;

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
          
          casa::Vector<double> guess(3);
          guess(0)=2;
          guess(1)=-3;
          guess(2)=5;
          itsIPWrong.add("poly", guess);

          itsPolyPerfect.reset(new PolynomialEquation(ipPerfect, itsDataPerfect, itsWeights, 
                               itsArguments, itsModelPerfect));
          itsPolyPerfect->predict();
          itsDataPerfect=itsModelPerfect.copy();
          itsPolyPerfect.reset(new PolynomialEquation(ipPerfect, itsDataPerfect, itsWeights, 
                               itsArguments, itsModelPerfect));
          itsPolyPerfect->predict();
           
          itsDataWrong = itsDataPerfect.copy();          
          itsPolyWrong.reset(new PolynomialEquation(itsIPWrong, itsDataWrong, itsWeights, itsArguments, itsModelWrong));
          itsPolyWrong->predict();
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
          // the data points used are far from perfect to discriminate between
          // two parabolas (need to go far from the origin), as a result we
          // need to increase the limit on the condition number.
          LinearSolver solver(1e5);
          GenericNormalEquations normeq;
          itsPolyWrong->calcEquations(normeq);
          solver.addNormalEquations(normeq);
          
          Quality q;
          solver.setAlgorithm("SVD");
          //std::cout << "Before " << itsIPWrong.value("poly") << std::endl;
          solver.solveNormalEquations(itsIPWrong,q);
          //std::cout << "After  " << itsIPWrong.value("poly") << std::endl;
          
          CPPUNIT_ASSERT(abs(q.cond()-11500.5)<1.0);
          const casa::Vector<double> result = itsIPWrong.value("poly");
          //std::cout<<result<<std::endl;
          CPPUNIT_ASSERT(fabs(result[0]-1.)<1e-5 && fabs(result[1]-2.)<1e-5 && fabs(result[2]-3.)<1e-5);
        }
        
        void testSolutionNEChol()
        {
          GenericNormalEquations normeq;
          itsPolyWrong->calcEquations(normeq);
          LinearSolver solver;
          solver.addNormalEquations(normeq);

          Quality q;
//          std::cout << "Before " << solver.parameters().value("poly") << std::endl;
          Params params = itsPolyWrong->parameters();
          solver.solveNormalEquations(params,q);
//          std::cout << "After  " << params.value("poly") << std::endl;
//          std::cout << q << std::endl;
        }

        void testComposite()
        {
          CompositeEquation comp;
          comp.add(*itsPolyPerfect);
          comp.predict();
          Params ip(comp.parameters());
          GenericNormalEquations normeq;//(ip);
          comp.calcEquations(normeq);
          casa::Vector<double> pvals(ip.value("poly").size());
          pvals.set(0.0);
          ip.update("poly", pvals);
          // data points are not good to discriminate between two parabolas,
          // condition number will be large, need to adjust the threshold
          // test here that a negative value specified by a static constant 
          // works and means no limit on the condition number
          LinearSolver solver(LinearSolver::KeepAllSingularValues);
          solver.addNormalEquations(normeq);
          Quality q;
          solver.setAlgorithm("SVD");
          solver.solveNormalEquations(ip,q);
          CPPUNIT_ASSERT(abs(q.cond()-11500.5)<1.0);
        }

    };

  }
}
