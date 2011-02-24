/// @file
///
/// @brief test of EigenDecompose wrapper(s)
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

#include <cppunit/extensions/HelperMacros.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <utils/EigenDecompose.h>

namespace askap {

namespace scimath {

class EigenDecomposeTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(EigenDecomposeTest);
   CPPUNIT_TEST(testSymEigenDecomp);
   CPPUNIT_TEST_SUITE_END();
public:
   void testSymEigenDecomp() {
      casa::Matrix<double> M(3,3,0.);
      M(0,0)=1.;
      M(1,1)=2.;
      M(2,2)=3.;
      M(0,1) = M(1,0) = -0.5;
      M(0,2) = M(2,0) = 0.3;
      M(1,2) = M(2,1) = 0.8;
      
      casa::Matrix<double> eVect;
      casa::Vector<double> eVal;
      symEigenDecompose(M,eVal,eVect);
      
      CPPUNIT_ASSERT(eVal.nelements() == M.nrow());
      CPPUNIT_ASSERT(eVect.nrow() == M.nrow());
      CPPUNIT_ASSERT(eVect.ncolumn() == M.ncolumn());
      CPPUNIT_ASSERT(M.nrow() == M.ncolumn());
      CPPUNIT_ASSERT(eVal[0] >= eVal[1]);
      CPPUNIT_ASSERT(eVal[1] >= eVal[2]);
      // checking eigenvalues and eigenvectors (M*vect = val*vect)
      for (casa::uInt i = 0; i<eVal.nelements(); ++i) {
           // checking i-th value and vector
           for (casa::uInt row=0; row<M.nrow(); ++row) {
                double res = 0;
                for (casa::uInt col=0; col<M.ncolumn(); ++col) {
                     res += M(row,col)*eVect(col,i);
                } 
                CPPUNIT_ASSERT_DOUBLES_EQUAL(eVect(row,i)*eVal[i],res,1e-6);
           }
      }
   };
};

} // namespace scimath

} // namespace askap

