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
#include <measurementequation/LeakageTerm.h>
#include <fitting/Params.h>

#include <fitting/LinearSolver.h>
#include <dataaccess/DataIteratorStub.h>
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
      CPPUNIT_TEST_SUITE_END();
     
     public:
      void setup() {
          itsIter = boost::shared_ptr<DataIteratorStub>(new DataIteratorStub(1));
          DataAccessorStub &da = dynamic_cast<DataAccessorStub&>(*itsIter);
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
               itsParams1->add("gain.g12."+toString(ant)+".0",
                            casa::Complex(realD[ant],imagD[ant]));
               itsParams1->add("gain.g21."+toString(ant)+".0",
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
               itsParams2->add("gain.g12."+toString(ant)+".0",0.);
               itsParams2->add("gain.g21."+toString(ant)+".0",0.);
          }
       
          itsCE2.reset(new ComponentEquation(*itsParams2, itsIter));
      
      }
     
      void testSolve() {} 
      
     private:
      typedef CalibrationME<LeakageTerm> METype;
      boost::shared_ptr<ComponentEquation> itsCE1, itsCE2;
      boost::shared_ptr<METype> itsEq1,itsEq2;
      boost::shared_ptr<scimath::Params> itsParams1, itsParams2;
      SharedIter<DataIteratorStub> itsIter;      
    };
  } // namespace synthesis

} // namespace askap


#endif // #ifndef POL_LEAKAGE_TEST_H



