/// @file
///
/// Tests gathered in this file are testing casacore,
/// rather than our code itself. It is largely to ensure that our interpretation of e.g.
/// image projection and coordinate conversion interfaces is correct and does not change if casacore, etc evolves
/// (i.e. because existing documentation has gaps)
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

#include <cppunit/extensions/HelperMacros.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/Projection.h>


#include <boost/shared_ptr.hpp>

#include <Common/ParameterSet.h>


namespace askap {

namespace scimath {

class CasaProjectionTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(CasaProjectionTest);
   CPPUNIT_TEST(testSINProjection);
   CPPUNIT_TEST_SUITE_END();
public:
   void setUp() {
      casa::Matrix<casa::Double> xform(2,2,0.);
      xform.diagonal() = 1.;
      const double deg2rad = casa::C::pi/180.;
      itsCoord.reset(new casa::DirectionCoordinate(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                     135*deg2rad, -60*deg2rad,
                     -1.*deg2rad, 1.*deg2rad,
                     xform, 128,128));
      itsWorld.resize(2);
      itsPixel.resize(2);               
   }
   
   void testSINProjection() {
      const double deg2rad = casa::C::pi/180.;
      itsPixel = 128.;
      toWorld();
      CPPUNIT_ASSERT(casa::abs(itsWorld[0]-135.*deg2rad)<1e-7);
      CPPUNIT_ASSERT(casa::abs(itsWorld[1]+60.*deg2rad)<1e-7); 
      const casa::Vector<casa::Double> referenceCentre = itsWorld.copy();
      itsPixel[0] = 100.;
      itsPixel[1] = 118.;
      toWorld();
      // doing conversion using straight formulas
      const double L = -(itsPixel[0] - 128.)*deg2rad;
      const double M = (itsPixel[1] - 128.)*deg2rad;
      CPPUNIT_ASSERT(L*L+M*M<1.);
      const double dec = asin(M*cos(referenceCentre[1])+sin(referenceCentre[1])*sqrt(1-L*L-M*M));
      const double ra = referenceCentre[0]+atan2(L, cos(referenceCentre[1])*sqrt(1-L*L-M*M)-M*sin(referenceCentre[1]));
      // check that the result is identical
      CPPUNIT_ASSERT(casa::abs(itsWorld[0]-ra)<1e-7);
      CPPUNIT_ASSERT(casa::abs(itsWorld[1]-dec)<1e-7);             
   }
protected:
   void toWorld() {
      CPPUNIT_ASSERT(itsCoord);
      CPPUNIT_ASSERT(itsWorld.nelements() == 2);
      CPPUNIT_ASSERT(itsPixel.nelements() == 2);
      CPPUNIT_ASSERT(itsCoord->toWorld(itsWorld,itsPixel));
      CPPUNIT_ASSERT(itsWorld.nelements() == 2);
   }   
private:
   boost::shared_ptr<casa::DirectionCoordinate>  itsCoord; 
   casa::Vector<casa::Double> itsWorld;
   casa::Vector<casa::Double> itsPixel;
};
    
} // namespace scimath

} // namespace askap

