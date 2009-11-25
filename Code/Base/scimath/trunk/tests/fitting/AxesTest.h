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

#include <fitting/Axes.h>
#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

#include <casa/Arrays/Matrix.h>

#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapError.h>
#include <coordinates/Coordinates/Projection.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

#include <vector>

namespace askap
{
  namespace scimath
  {

    class AxesTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(AxesTest);
      CPPUNIT_TEST(testIndices);
      CPPUNIT_TEST(testValues);
      CPPUNIT_TEST_EXCEPTION(testDuplError, askap::CheckError);
      CPPUNIT_TEST(testUpdate);
      CPPUNIT_TEST(testCopy);
      CPPUNIT_TEST(testStokes);
      CPPUNIT_TEST(testDirection);
      CPPUNIT_TEST_SUITE_END();

      private:
        boost::shared_ptr<Domain> p1, p2, p3, pempty;

      public:
        void setUp()
        {
          p1.reset(new Domain());
          p2.reset(new Domain());
          p3.reset(new Domain());
          pempty.reset(new Domain());
        }

        void testDuplError()
// Should throw Invalid argument
        {
          p1->add("Time", 0.0, 1.0);
          p1->add("Time", 0.0, 1.0);
        }

        void testCopy()
        {
          CPPUNIT_ASSERT( !p1->has("Time"));
          p1->add("Time", 0.0, 1.0);
          CPPUNIT_ASSERT(p1->has("Time"));
          p1->add("Freq", 0.7e9, 1.7e9);
          Domain pnew(*p1);
          p1->update("Time",-10.,10.);

          CPPUNIT_ASSERT(pnew.has("Time"));
          CPPUNIT_ASSERT(pnew.start("Time")==0.0);
          CPPUNIT_ASSERT(pnew.order("Time")==0);
          CPPUNIT_ASSERT(pnew.end("Time")==1.0);

          CPPUNIT_ASSERT(pnew.has("Freq"));
          CPPUNIT_ASSERT(pnew.start("Freq")==0.7e9);
          CPPUNIT_ASSERT(pnew.order("Freq")==1);
          CPPUNIT_ASSERT(pnew.end("Freq")==1.7e9);
        }

        void testValues()
        {
          CPPUNIT_ASSERT( !p1->has("Time"));
          p1->add("Time", 0.0, 1.0);
          CPPUNIT_ASSERT(p1->has("Time"));
          p1->add("Freq", 0.7e9, 1.7e9);

          CPPUNIT_ASSERT(p1->has("Time"));
          CPPUNIT_ASSERT(p1->start("Time")==0.0);
          CPPUNIT_ASSERT(p1->end("Time")==1.0);

          CPPUNIT_ASSERT(p1->has("Freq"));
          CPPUNIT_ASSERT(p1->start("Freq")==0.7e9);
          CPPUNIT_ASSERT(p1->end("Freq")==1.7e9);
        }
        
        void testUpdate()
        {
          CPPUNIT_ASSERT( !p1->has("Time"));
          p1->update("Time", -10.0, 10.0);
          CPPUNIT_ASSERT(p1->has("Time"));
          
          p1->update("Time",0.0, 1.0);
          CPPUNIT_ASSERT(p1->has("Time"));
          CPPUNIT_ASSERT(casa::abs(p1->start("Time"))<1e-7);
          CPPUNIT_ASSERT(casa::abs(p1->end("Time")-1.0)<1e-7);
                    
          p1->update("Freq", 0.7e9, 1.7e9);

          CPPUNIT_ASSERT(p1->has("Freq"));
          CPPUNIT_ASSERT(casa::abs(p1->start("Freq")-0.7e9)<1);
          CPPUNIT_ASSERT(casa::abs(p1->end("Freq")-1.7e9)<1);          
        }

        void testIndices()
        {
          CPPUNIT_ASSERT(!p1->has("Time"));
          p1->add("Time", 0.0, 1.0);
          CPPUNIT_ASSERT(p1->has("Time"));
          p1->add("Freq", 0.7e9, 1.7e9);
        }

        void testStokes()
        {
          casa::Vector<casa::Stokes::StokesTypes> stokes(4);
          stokes[0] = casa::Stokes::I;
          stokes[1] = casa::Stokes::Q;
          stokes[2] = casa::Stokes::U;
          stokes[3] = casa::Stokes::V;
          doStokesTest(stokes);
          
          stokes.resize(1);
          stokes[0] = casa::Stokes::I;
          doStokesTest(stokes);
                    
          stokes.resize(2);
          stokes[0] = casa::Stokes::XX;
          stokes[1] = casa::Stokes::YY;
          doStokesTest(stokes);

          stokes.resize(2);
          stokes[0] = casa::Stokes::RR;
          stokes[1] = casa::Stokes::RL;
          doStokesTest(stokes);
        }
        
        void doStokesTest(const casa::Vector<casa::Stokes::StokesTypes> &stokes) {
          p1->addStokesAxis(stokes);
          casa::Vector<casa::Stokes::StokesTypes> res = p1->stokesAxis();
          CPPUNIT_ASSERT(res.nelements() == stokes.nelements());
          for (size_t pol = 0; pol<stokes.nelements(); ++pol) {
               CPPUNIT_ASSERT(stokes[pol] == res[pol]);
          }
        }
        
        void testDirection()
        {
          CPPUNIT_ASSERT(!p1->hasDirection());
          
          casa::Matrix<casa::Double> xform(2,2,0.);
          xform.diagonal() = 1.;
          const double deg2rad = casa::C::pi/180.;
          casa::DirectionCoordinate  dc(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                     135*deg2rad, -60*deg2rad,-1.*deg2rad, 1.*deg2rad, xform, 128,128);
          
          p1->addDirectionAxis(dc);
          CPPUNIT_ASSERT(p1->hasDirection());
          CPPUNIT_ASSERT(dc.near(p1->directionAxis()));
          casa::DirectionCoordinate  dc2(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                     134.9*deg2rad, -60.1*deg2rad,-0.9*deg2rad, 1.*deg2rad, xform, 127,129);
          CPPUNIT_ASSERT(!dc2.near(p1->directionAxis()));
          p1->addDirectionAxis(dc2);
          CPPUNIT_ASSERT(p1->hasDirection());
          CPPUNIT_ASSERT(dc2.near(p1->directionAxis()));
          CPPUNIT_ASSERT(!dc.near(p1->directionAxis()));
          // check I/O
          LOFAR::BlobString b1(false);
          LOFAR::BlobOBufString bob(b1);
          LOFAR::BlobOStream bos(bob);
          bos << *p1;
          
          CPPUNIT_ASSERT(!p2->hasDirection());
          LOFAR::BlobIBufString bib(b1);
          LOFAR::BlobIStream bis(bib);
          bis >> *p2;
          
          CPPUNIT_ASSERT(p2->hasDirection());
          CPPUNIT_ASSERT(dc2.near(p2->directionAxis()));
          
          p2->addDirectionAxis(dc);
          CPPUNIT_ASSERT(!dc2.near(p2->directionAxis()));
          
          // test copy of the direction axis
          Axes newAxes(*p2);
          CPPUNIT_ASSERT(newAxes.hasDirection());
          CPPUNIT_ASSERT(dc.near(newAxes.directionAxis()));
          // check that there is no referencing
          p2->addDirectionAxis(dc2);
          CPPUNIT_ASSERT(dc.near(newAxes.directionAxis()));
          // test assignment
          CPPUNIT_ASSERT(!p3->hasDirection());
          *p3 = newAxes;
          CPPUNIT_ASSERT(p3->hasDirection());
          CPPUNIT_ASSERT(dc.near(p3->directionAxis()));
          // check that there is no referencing
          newAxes.addDirectionAxis(dc2);
          CPPUNIT_ASSERT(dc.near(p3->directionAxis()));          
        }
        
    };

  }
}
