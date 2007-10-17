/// @file
/// 
/// @brief This file contains general fitting tests
/// @details One of the examples is an equation, which appear in
/// the gain calibration problem (Vmeas= g1*conj(g2)*Vtrue).
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef GENERAL_FITTING_TEST_H
#define GENERAL_FITTING_TEST_H

#include <fitting/LinearSolver.h>

#include <cppunit/extensions/HelperMacros.h>

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>

#include <cmath>

using std::abs;

namespace conrad
{

namespace scimath
{

  class GeneralFittingTest : public CppUnit::TestFixture {

     CPPUNIT_TEST_SUITE(GeneralFittingTest);
     CPPUNIT_TEST(testRealEquation);
     CPPUNIT_TEST_SUITE_END();

  public:
     void setUp() {
        itsNAnt = 6;
        const casa::uInt nBaselines = itsNAnt*(itsNAnt-1)/2;
        itsBaselines.resize(nBaselines);
        for (casa::uInt ant1=0,baseline=0;ant1<itsNAnt;++ant1) {
             for (casa::uInt ant2=ant1+1;ant2<itsNAnt;++ant2,++baseline) {
                  CONRADASSERT(baseline<nBaselines);
                  itsBaselines[baseline].first=ant1;
                  itsBaselines[baseline].second=ant2;
             }
        }
        
     }

     void testRealEquation() {
         const double trueGains[] = {0.9,1.1,1.2,0.6,1.3,1.05}; 
         createParams(&trueGains[0],itsTrueGains);
         predictReal();
         const double guessedGains[] = {1.,1.,1.,1.,1.,1.};
         createParams(&guessedGains[0],itsGuessedGains);
         NormalEquations ne(itsGuessedGains);
         calcEquationsReal(ne);
         Quality q;
         LinearSolver solver(itsGuessedGains);
         solver.addNormalEquations(ne);
         solver.setAlgorithm("SVD");
         solver.solveNormalEquations(q);  
         for (casa::uInt ant=0;ant<itsNAnt;++ant) {
              CPPUNIT_ASSERT(abs(itsGuessedGains.scalarValue(parName(ant))-
                                 trueGains[ant])<0.05);
         }     
     }
     
  protected:
     /// @brief create parameter object from a sequence of values
     /// @details This method reads itsNAnt elements from the specified
     /// sequence (general iterator semantics) and packs them into 
     /// a Param object. Template is used to allow various types of
     /// input iterators to be used. Because Param::add is an overloaded
     /// method, the template approach also ensures that the correct 
     /// add method will be used for each individual type of the input
     /// values (i.e. complex or real will be determined automatically)
     /// @param[in] iter input iterator of type Iter (template parameter).
     /// The method will read exactly itsNAnt values.
     /// @param[out] params a reference to Params object to fill (will be reset) 
     template<typename Iter> 
     void createParams(const Iter &iter, Params &params) const {
          params.reset();
          Iter cIter(iter);
          for (casa::uInt ant=0; ant<itsNAnt; ++ant,++cIter) {
               params.add(parName(ant),*cIter);
          }
     }
  
     /// @brief obtain parameter name from antenna number
     /// @details
     /// @param[in] ant antenna number
     /// @return string with parameter name
     std::string parName(casa::uInt ant) const {
         CONRADASSERT(ant<itsNAnt);
         return std::string("gain.")+utility::toString(ant);
     }
     
     /// @brief obtain antenna number from the parameter name
     /// @details Exception is thrown, if parameter name is incompatible
     /// @param[in] par parameter name
     /// @return antenna number [0..itsNAnt-1]
     casa::uInt antNumber(const std::string &par) const {
         CONRADASSERT(par.find("gain.") == 0 && par.size()>5);
         casa::uInt ant = utility::fromString<casa::uInt>(par.substr(5));
         CONRADASSERT(ant<itsNAnt);
         return ant;
     }
     
     /// @brief predict products from itsTrueGains
     /// @details This variant implements equation working with
     /// real-valued gains. Predicted values are stored in itsRealMeasuredValues
     void predictReal() {
         itsRealMeasuredValues.resize(itsBaselines.size());
         for (size_t baseline=0; baseline<itsBaselines.size(); ++baseline) {
               itsRealMeasuredValues[baseline] = 
                  itsTrueGains.scalarValue(parName(itsBaselines[baseline].first))*
                  itsTrueGains.scalarValue(parName(itsBaselines[baseline].second));
         }
     }
     
     /// @brief calculate normal equations at itsGuessedGains
     /// @details This variant calculates normal equations for the case
     /// of real-valued gains (i.e. just amplitudes). 
     /// @param[in] ne a reference to normal equations object
     void calcEquationsReal(NormalEquations &ne) {
         CONRADASSERT(itsBaselines.size() == itsRealMeasuredValues.size());
         casa::Matrix<double> derivatives(itsBaselines.size(),itsNAnt,0.);
         for (size_t baseline=0; baseline<itsBaselines.size(); ++baseline) {
              CONRADASSERT(itsBaselines[baseline].first<itsNAnt);
              CONRADASSERT(itsBaselines[baseline].second<itsNAnt);
              derivatives(baseline,itsBaselines[baseline].first) = 
                    itsGuessedGains.scalarValue(parName(itsBaselines[baseline].second));
              derivatives(baseline,itsBaselines[baseline].second) = 
                    itsGuessedGains.scalarValue(parName(itsBaselines[baseline].first));
         }
         DesignMatrix designMatrix(itsGuessedGains);
         for (casa::uInt ant=0; ant<itsNAnt; ++ant) {
              designMatrix.addDerivative(parName(ant),derivatives.column(ant));
         }
         casa::Vector<double> residual(itsRealMeasuredValues.copy());
         for (size_t baseline=0; baseline<itsBaselines.size(); ++baseline) {
              residual[baseline] -= 
                 itsGuessedGains.scalarValue(parName(itsBaselines[baseline].first))*
                 itsGuessedGains.scalarValue(parName(itsBaselines[baseline].second)); 
         }
         designMatrix.addResidual(residual,casa::Vector<double>(residual.size(),1.));
         ne.add(designMatrix);
     }
       

  private:
     /// @brief guessed parameters
     Params itsGuessedGains;
     /// @brief true parameters
     Params itsTrueGains;
     /// @brief data sample playing a role of measured data
     casa::Vector<double> itsRealMeasuredValues;     
     /// @brief first and second "antenna" corresponding to a baseline
     std::vector<std::pair<casa::uInt, casa::uInt> > itsBaselines;
     /// @brief number of antennae
     casa::uInt itsNAnt;
};

} // namespace scimath

} // namespace conrad

#endif // #ifndef GENERAL_FITTING_TEST_H
