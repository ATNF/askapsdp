/// @file
/// $brief Tests of the spheroidal function calculator
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
/// 
#ifndef SPHEROIDAL_FUNCTION_TEST_H
#define SPHEROIDAL_FUNCTION_TEST_H

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>
// own includes
#include <utils/SpheroidalFunction.h>
#include <askap/AskapError.h>
//#include <fstream>

namespace askap {

namespace scimath {

class SpheroidalFunctionTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SpheroidalFunctionTest);
  CPPUNIT_TEST(cmpValuesTest);
  CPPUNIT_TEST_SUITE_END();
public:
  void cmpValuesTest() {
      // c=pi*m/2, alpha = 1
      SpheroidalFunction sph(casa::C::pi*3, 1);
      const size_t nPt = 100;
      //std::ofstream os("dbg.dat");
      // deliberately avoid nu = +/- 1 points
      for (size_t i=1; i<nPt; ++i) {
           const double nu = -1. + 2./double(nPt)*double(i);
           // experiments show that the rational approximation is good down to
           // 1e-6, the following condition fails, if threshold is 1e-7 or lower
           CPPUNIT_ASSERT(fabs(grdsf(fabs(nu))-sph(nu))<1e-6);
      }
  }
protected:
    // find spheroidal function with m = 6, alpha = 1 using the rational
    // approximations discussed by fred schwab in 'indirect imaging'.
    // this routine was checked against fred's sphfn routine, and agreed
    // to about the 7th significant digit.
    // the gridding function is (1-nu**2)*grdsf(nu) where nu is the distance
    // to the edge. the grid correction function is just 1/grdsf(nu) where nu
    // is now the distance to the edge of the image.
    double grdsf(double nu) {
         double top, bot, delnusq, nuend;
         int k, part;
         int np = 4, nq = 2;
         double p[2][5] =
             { {8.203343e-2, -3.644705e-1, 6.278660e-1, -5.335581e-1, 2.312756e-1},
               {4.028559e-3, -3.697768e-2, 1.021332e-1, -1.201436e-1, 6.412774e-2}};
         double q[2][3]=
             { {1.0000000, 8.212018e-1, 2.078043e-1}, {1.0000000, 9.599102e-1,
                2.918724e-1}};
         double value = 0.0;

         if ((nu>=0.0)&&(nu<0.75)) {
             part = 0;
             nuend = 0.75;
         } else if ((nu>=0.75)&&(nu<=1.00)) {
             part = 1;
             nuend = 1.00;
         } else {
             value = 0.0;
             return value;
         }

         top = p[part][0];
         bot = q[part][0];
         delnusq = std::pow(nu, 2) - std::pow(nuend, 2);
         for (k = 1; k<= np; k++) {
              double factor=std::pow(delnusq, k);
              top += p[part][k] * factor;
         }
         for (k = 1; k<= nq; k++)  {
              double factor=std::pow(delnusq, k);
              bot += q[part][k] * factor;
         }
         if (bot!=0.0) {
             value = top/bot;
         } else  {
             value = 0.0;
         } if (value<0.0) {
               value=0.0;
         }
         return value;
    }
};
  
} // namespace scimath

} // namespace askap

#endif // #ifndef SPHEROIDAL_FUNCTION_TEST_H

