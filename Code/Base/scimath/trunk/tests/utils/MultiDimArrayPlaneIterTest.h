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

#include <utils/MultiDimArrayPlaneIter.h>
#include <askap/AskapUtil.h>

namespace askap {

namespace scimath {

   class MultiDimArrayPlaneIterTest : public CppUnit::TestFixture
    {
           CPPUNIT_TEST_SUITE(MultiDimArrayPlaneIterTest);
           CPPUNIT_TEST(testIteration);
           CPPUNIT_TEST_SUITE_END();
       public:
           void setUp() {
               itsArray.resize(casa::IPosition(5,2,2,4,3,1));
               double value = 0.;
               itsTags.resize(12);
               for (casa::uInt chan = 0; chan<3; ++chan) {
                    for (casa::uInt pol = 0; pol<4; ++pol) {
                         casa::Matrix<double> mtr(itsArray(casa::IPosition(5,0,0,pol,chan,0),
                                  casa::IPosition(5,1,1,pol,chan,0)).nonDegenerate());
                         mtr.set(value);
                         value += 1.;
                         itsTags[chan*4+pol] = std::string(".pol")+utility::toString<casa::uInt>(pol)+
                                     ".chan"+utility::toString<casa::uInt>(chan);
                    }
               }
           }
           
           void testIteration() {
              casa::uInt counter = 0;
              for (MultiDimArrayPlaneIter iter(itsArray.shape()); iter.hasMore(); iter.next(),++counter) {
                   CPPUNIT_ASSERT(counter<12);
                   CPPUNIT_ASSERT( iter.tag() == itsTags[counter]);
                   CPPUNIT_ASSERT( iter.planeShape() == casa::IPosition(5,2,2,1,1,1));
                   CPPUNIT_ASSERT( iter.shape() == casa::IPosition(5,2,2,4,3,1));
                   CPPUNIT_ASSERT( iter.position() == casa::IPosition(5,0,0,counter % 4, counter/4,0));
                   CPPUNIT_ASSERT( iter.sequenceNumber() == counter);
                   casa::Array<double> plane = iter.getPlane(itsArray);
                   CPPUNIT_ASSERT( plane.shape().nonDegenerate().nelements() == 2 );
                   casa::Matrix<double> mtr(plane.nonDegenerate());
                   casa::Vector<double> flattened = itsArray.reform(casa::IPosition(1,itsArray.nelements()));
                   casa::Array<double> plane2 = iter.getPlane(flattened);
                   CPPUNIT_ASSERT( plane2.shape() == iter.planeShape() );
                   CPPUNIT_ASSERT( plane2.shape().nonDegenerate().nelements() == 2 );
                   casa::Matrix<double> mtr2(plane2.nonDegenerate());
                   
                   for (casa::uInt row = 0; row<mtr.nrow(); ++row) {
                        for (casa::uInt col = 0; col<mtr.ncolumn(); ++col) {
                             CPPUNIT_ASSERT(row<mtr2.nrow());
                             CPPUNIT_ASSERT(col<mtr2.ncolumn());
                             CPPUNIT_ASSERT(fabs(mtr(row,col)-double(counter))<1e-6);
                             CPPUNIT_ASSERT(fabs(mtr2(row,col)-double(counter))<1e-6);
                        }
                   }     
                   
              }
              CPPUNIT_ASSERT( counter == 12);
              
           }
       private:
           casa::Array<double> itsArray;
           std::vector<string> itsTags;
     };
     
} // namespace scimath

} // namespace askap


#endif // #ifndef MULTI_DIM_ARRAY_PLANE_ITER_TEST_H

