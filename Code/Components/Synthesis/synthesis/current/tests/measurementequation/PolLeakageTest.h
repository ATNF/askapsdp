/// @file
/// 
/// @brief Unit tests for polarisation leakage calibration.
/// @details The tests gathered in this file predict visibility data
/// with some calibration errors and then solve for them.
/// 
///
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef POL_LEAKAGE_TEST_H
#define POL_LEAKAGE_TEST_H

#include <measurementequation/ComponentEquation.h>
#include <measurementequation/CalibrationME.h>
#include <measurementequation/PreAvgCalMEBase.h>
#include <measurementequation/LeakageTerm.h>
#include <fitting/Params.h>

#include <fitting/LinearSolver.h>
#include <dataaccess/DataIteratorStub.h>
#include <measurementequation/CalibrationApplicatorME.h>
#include <calibaccess/CachedCalSolutionAccessor.h>
#include <calibaccess/CalSolutionSourceStub.h>
#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <boost/shared_ptr.hpp>


namespace askap
{
  namespace synthesis
  {

    class PolLeakageTest : public CppUnit::TestFixture
    {
      CPPUNIT_TEST_SUITE(PolLeakageTest);
      CPPUNIT_TEST(testSolve);
      CPPUNIT_TEST(testSolvePreAvg);
      CPPUNIT_TEST(testApplication);            
      CPPUNIT_TEST_SUITE_END();
     
     public:
      void setUp() {
          itsIter = boost::shared_ptr<accessors::DataIteratorStub>(new accessors::DataIteratorStub(1));
          accessors::DataAccessorStub &da = dynamic_cast<accessors::DataAccessorStub&>(*itsIter);
          ASKAPASSERT(da.itsStokes.nelements() == 1);
          
          casa::Vector<casa::Stokes::StokesTypes> stokes(4);
          stokes[0] = casa::Stokes::XX;
          stokes[1] = casa::Stokes::XY;
          stokes[2] = casa::Stokes::YX;
          stokes[3] = casa::Stokes::YY;         
          
          da.itsStokes.assign(stokes.copy());
          da.itsVisibility.resize(da.nRow(), 2 ,4);
          da.itsVisibility.set(casa::Complex(-10.,15.));
          da.itsNoise.resize(da.nRow(),da.nChannel(),da.nPol());
          da.itsNoise.set(1.);
          da.itsFlag.resize(da.nRow(),da.nChannel(),da.nPol());
          da.itsFlag.set(casa::False);
          da.itsFrequency.resize(da.nChannel());
          for (casa::uInt ch = 0; ch < da.nChannel(); ++ch) {
               da.itsFrequency[ch] = 1.4e9 + 20e6*double(ch);
          }
                    
          const casa::uInt nAnt = 30;
          
          // leakages, assume g12=g21
          const double realD[nAnt] = {0.1, -0.1, 0.05, -0.13, 0.333,
                                          0.1, 0.0, 0.0, -0.2, 0.03, 
                                         -0.05, 0.1, -0.1, -0.02, 0.03,
                                         -0.03, -0.1, 0.1, 0.1, 0.05,
                                          0.0, -0.03, 0.1, 0.03, 0.08,
                                          0.05, -0.07, 0.054, 0.0, 0.1}; 
          const double imagD[nAnt] = {0.0, 0., -0.05, 0.0587, 0.,
                                          0., -0.1, 0.02, -0.1, 0.84, 
                                          0.086, 0.1, 0.1, 0., 0.03,
                                         -0.084, 0., 0., -0.1, -0.05,
                                          0.02, 0.09, 0.1, 0.03, -0.1,
                                         -0.09, 0.072, -0.04, 0.05, -0.1}; 
      
          itsParams1.reset(new scimath::Params);
          itsParams1->add("flux.i.cena", 100.);
          itsParams1->add("direction.ra.cena", 0.5*casa::C::arcsec);
          itsParams1->add("direction.dec.cena", -0.3*casa::C::arcsec);
          itsParams1->add("shape.bmaj.cena", 3.0e-3*casa::C::arcsec);
          itsParams1->add("shape.bmin.cena", 2.0e-3*casa::C::arcsec);
          itsParams1->add("shape.bpa.cena", -55*casa::C::degree);
          for (casa::uInt ant=0; ant<nAnt; ++ant) {
               itsParams1->add("leakage.d12."+toString(ant)+".0",
                            casa::Complex(realD[ant],imagD[ant]));
               itsParams1->add("leakage.d21."+toString(ant)+".0",
                            casa::Complex(realD[ant],imagD[ant]));
          }
          //

          itsCE1.reset(new ComponentEquation(*itsParams1, itsIter));
          itsEq1.reset(new METype(*itsParams1,itsIter,itsCE1));
      
          
          itsParams2.reset(new scimath::Params);
          itsParams2->add("flux.i.cena", 100.);
          itsParams2->add("direction.ra.cena", 0.50000*casa::C::arcsec);
          itsParams2->add("direction.dec.cena", -0.30000*casa::C::arcsec);
          itsParams2->add("shape.bmaj.cena", 3.0e-3*casa::C::arcsec);
          itsParams2->add("shape.bmin.cena", 2.0e-3*casa::C::arcsec);
          itsParams2->add("shape.bpa.cena", -55*casa::C::degree);
          for (casa::uInt ant=0; ant<nAnt; ++ant) {
               itsParams2->add("leakage.d12."+toString(ant)+".0",casa::Complex(0.));
               itsParams2->add("leakage.d21."+toString(ant)+".0",casa::Complex(0.));
          }
       
          itsCE2.reset(new ComponentEquation(*itsParams2, itsIter));
      
      }
     
      void testSolve() {
          // Predict with the "perfect" parameters"
          CPPUNIT_ASSERT(itsEq1);
          itsEq1->predict();
          std::vector<std::string> freeNames = itsParams2->freeNames();
          for (std::vector<std::string>::const_iterator it = freeNames.begin();
               it!=freeNames.end();++it) {
               if (it->find("leakage") == std::string::npos) {
                   itsParams2->fix(*it);
               }
          }
          
          for (size_t iter=0; iter<5; ++iter) {
               // Calculate gradients using "imperfect" parameters"
               GenericNormalEquations ne; //(*params2);
            
               itsEq2.reset(new METype(*itsParams2,itsIter,itsCE2));
            
               itsEq2->calcEquations(ne);
               Quality q;
               LinearSolver solver1;
               solver1.addNormalEquations(ne);
               solver1.setAlgorithm("SVD");
               solver1.solveNormalEquations(*itsParams2,q);  
               //std::cout<<q<<std::endl;               
          }
          
          freeNames = itsParams2->freeNames();
          for (std::vector<std::string>::const_iterator it = freeNames.begin();
               it!=freeNames.end();++it) {
               CPPUNIT_ASSERT(itsParams2->has(*it));
               CPPUNIT_ASSERT(itsParams1->has(*it));               
               CPPUNIT_ASSERT_DOUBLES_EQUAL(casa::abs(itsParams2->complexValue(*it) -
                                            itsParams1->complexValue(*it)), 0., 1e-6);
          }
      } 
      
      void testSolvePreAvg() {
          // Predict with the "perfect" parameters"
          CPPUNIT_ASSERT(itsEq1);
          itsEq1->predict();
          std::vector<std::string> freeNames = itsParams2->freeNames();
          for (std::vector<std::string>::const_iterator it = freeNames.begin();
               it!=freeNames.end();++it) {
               if (it->find("leakage") == std::string::npos) {
                   itsParams2->fix(*it);
               }
          }
    
          typedef CalibrationME<LeakageTerm,PreAvgCalMEBase> PreAvgMEType;
          boost::shared_ptr<PreAvgMEType> preAvgEq(new PreAvgMEType());
          CPPUNIT_ASSERT(preAvgEq);
          preAvgEq->accumulate(itsIter,itsCE2);

          for (size_t iter=0; iter<5; ++iter) {
               // Calculate gradients using "imperfect" parameters"
               GenericNormalEquations ne; 
            
               preAvgEq->setParameters(*itsParams2);            
               preAvgEq->calcEquations(ne);
               
               Quality q;
               LinearSolver solver1;
               solver1.addNormalEquations(ne);
               solver1.setAlgorithm("SVD");
               solver1.solveNormalEquations(*itsParams2,q);  
          }

          freeNames = itsParams2->freeNames();
          for (std::vector<std::string>::const_iterator it = freeNames.begin();
               it!=freeNames.end();++it) {
               CPPUNIT_ASSERT(itsParams2->has(*it));
               CPPUNIT_ASSERT(itsParams1->has(*it));                              
               CPPUNIT_ASSERT_DOUBLES_EQUAL(0.,casa::abs(itsParams2->complexValue(*it) -
                                            itsParams1->complexValue(*it)), 1e-6);               
          }                   
      }
      void testApplication() {        
          // check that everything is set up for full stokes
          CPPUNIT_ASSERT(itsIter);
          accessors::DataAccessorStub &da = dynamic_cast<accessors::DataAccessorStub&>(*itsIter);          
          CPPUNIT_ASSERT(da.itsStokes.nelements() == 4);
          da.rwVisibility().set(0.);          
          
          const casa::uInt nAnt = 30;
          // use the following values to form both gains and leakages
          const double realGains[nAnt] = {1.1, 0.9, 1.05, 0.87, 1.333,
                                          1.1, 1.0, 1.0, -1.0, 0.3, 
                                         -0.5, 1.1, 0.9, 0.98, 1.03,
                                         -0.3, -1.1, 0.9, 1.1, 1.05,
                                          1.0, -0.3, 1.1, 0.3, 1.8,
                                          0.5, -0.7, 1.054, 1.0, 1.1}; 
          const double imagGains[nAnt] = {0.0, 0., -0.05, 0.587, 0.,
                                          0., -0.1, 0.02, -0.1, 0.84, 
                                          0.86, 0.1, 0.1, 0., 0.03,
                                         -0.84, 0., 0., -0.1, -0.05,
                                          0.2, 0.9, 1.1, 0.3, -0.1,
                                         -0.9, 0.72, -0.04, 0.05, -0.1}; 
          
          itsParams1.reset(new scimath::Params);
          itsParams1->add("flux.i.cena", 1.);
          itsParams1->add("direction.ra.cena", 0.*casa::C::arcsec);
          itsParams1->add("direction.dec.cena", 0.*casa::C::arcsec);
          for (casa::uInt ant=0; ant<nAnt; ++ant) {
               itsParams1->add(accessors::CalParamNameHelper::paramName(ant,0,casa::Stokes::XX),
                            casa::Complex(realGains[ant],imagGains[ant]));
               itsParams1->add(accessors::CalParamNameHelper::paramName(ant,0,casa::Stokes::YY),
                            casa::Complex(realGains[nAnt - 1 - ant],imagGains[nAnt - 1 - ant]));
                            /*
               itsParams1->add(accessors::CalParamNameHelper::paramName(ant,0,casa::Stokes::XY),
                            casa::Complex(realGains[ant] - 1.,imagGains[ant])/casa::Complex(10.,0.));
               itsParams1->add(accessors::CalParamNameHelper::paramName(ant,0,casa::Stokes::YX),
                            casa::Complex(realGains[ant] - 1.,imagGains[ant])/casa::Complex(10.,0.));                                           
                            */
          }

          itsCE1.reset(new ComponentEquation(*itsParams1, itsIter));
          //typedef CalibrationME<Product<NoXPolGain,LeakageTerm> > METype2;
          typedef CalibrationME<NoXPolGain> METype2;
          
          boost::shared_ptr<METype2> eq1(new METype2(*itsParams1,itsIter,itsCE1));
          eq1->predict();
          
          accessors::CachedCalSolutionAccessor acc(itsParams1);
          accessors::CalSolutionSourceStub src(boost::shared_ptr<accessors::CachedCalSolutionAccessor>(&acc,utility::NullDeleter()));
          CalibrationApplicatorME calME(boost::shared_ptr<accessors::CalSolutionSourceStub>(&src,utility::NullDeleter()));
          calME.correct(da);

          // check visibilities after calibration application
          const casa::Cube<casa::Complex>& vis = da.visibility();
          for (casa::uInt row = 0; row < da.nRow(); ++row) {
               for (casa::uInt chan = 0; chan < da.nChannel(); ++chan) {
                    for (casa::uInt pol = 0; pol < da.nPol(); ++pol) {
                         //std::cout<<row<<" "<<da.antenna1()[row]<<" "<<da.antenna2()[row]<<" "<<vis(row,chan,pol)<<" "<<chan<<" "<<pol<<std::endl;
                         CPPUNIT_ASSERT_DOUBLES_EQUAL(pol % 3 == 0 ? 1. : 0., real(vis(row,chan,pol)),1e-1);
                         CPPUNIT_ASSERT_DOUBLES_EQUAL(0., imag(vis(row,chan,pol)),1e-1);                         
                    }
               }
          }
        }
      
      
     private:
      typedef CalibrationME<LeakageTerm> METype;
      boost::shared_ptr<ComponentEquation> itsCE1, itsCE2;
      boost::shared_ptr<METype> itsEq1,itsEq2;
      boost::shared_ptr<scimath::Params> itsParams1, itsParams2;
      accessors::SharedIter<accessors::DataIteratorStub> itsIter;      
    };
  } // namespace synthesis

} // namespace askap


#endif // #ifndef POL_LEAKAGE_TEST_H



