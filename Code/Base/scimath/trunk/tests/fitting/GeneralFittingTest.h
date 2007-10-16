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
        CPPUNIT_ASSERT(true);  
     }

  private:
     /// @brief guessed parameters
     Params itsGuess;
     /// @brief data sample playing a role of measured data
     casa::Vector<double> itsMeasuredValues;     
     /// @brief first and second "antenna" corresponding to a baseline
     std::vector<std::pair<casa::uInt, casa::uInt> > itsBaselines;
     /// @brief number of antennae
     casa::uInt itsNAnt;
};

} // namespace scimath

} // namespace conrad

#endif // #ifndef GENERAL_FITTING_TEST_H
