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

#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapError.h>
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
      CPPUNIT_TEST_SUITE_END();

      private:
        Domain *p1, *p2, *p3, *pempty;

      public:
        void setUp()
        {
          p1 = new Domain();
          p2 = new Domain();
          p3 = new Domain();
          pempty = new Domain();
        }

        void tearDown()
        {
          delete p1;
          delete p2;
          delete p3;
          delete pempty;
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
          p1->addStokesAxis(stokes);
          casa::Vector<casa::Stokes::StokesTypes> res = p1->stokesAxis();
          CPPUNIT_ASSERT(res.nelements() == stokes.nelements());
          for (size_t pol = 0; pol<stokes.nelements(); ++pol) {
               CPPUNIT_ASSERT(stokes[pol] == res[pol]);
          }
        }
    };

  }
}
