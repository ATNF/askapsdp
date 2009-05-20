/// @file
/// 
/// @brief Unit tests for MultiDimArrayPlaneIter.
/// @details MultiDimArrayPlaneIter is an iterator class designed to facilitate processing
/// of hypercube inside the solver.
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

#ifndef MULTI_DIM_ARRAY_PLANE_ITER_TEST_H
#define MULTI_DIM_ARRAY_PLANE_ITER_TEST_H

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>

#include <cppunit/extensions/HelperMacros.h>

#include <measurementequation/MultiDimArrayPlaneIter.h>

namespace askap {

namespace synthesis {

   class MultiDimArrayPlaneIterTest : public CppUnit::TestFixture
    {
           CPPUNIT_TEST_SUITE(MultiDimArrayPlaneIterTest);
           CPPUNIT_TEST(testIteration);
           CPPUNIT_TEST_SUITE_END();
       public:
           void setUp() {
               itsArray.resize(casa::IPosition(5,2,2,4,3,1));
               double value = 0.;
               for (casa::uInt pol = 0; pol<4; ++pol) {
                    for (casa::uInt chan = 0; chan<3; ++chan) {
                         casa::Matrix<double> mtr(itsArray(casa::IPosition(5,0,0,pol,chan,0),
                                  casa::IPosition(5,1,1,pol,chan,0)).nonDegenerate());
                         mtr.set(value);
                         value += 1.;
                    }
               }
           }
           
           void testIteration() {
           }
       private:
           casa::Array<double> itsArray;
     };
     
} // namespace synthesis

} // namespace askap


#endif // #ifndef MULTI_DIM_ARRAY_PLANE_ITER_TEST_H

