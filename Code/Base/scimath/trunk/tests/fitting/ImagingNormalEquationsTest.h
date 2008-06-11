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

#include <fitting/ImagingNormalEquations.h>

#include <cppunit/extensions/HelperMacros.h>

#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>


#include <askap/AskapError.h>


namespace askap
{
  namespace scimath
  {

    class ImagingNormalEquationsTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(ImagingNormalEquationsTest);
      CPPUNIT_TEST(testConstructors);
      CPPUNIT_TEST(testCopy);
      CPPUNIT_TEST(testBlobStream);
      CPPUNIT_TEST_SUITE_END();

      private:
        ImagingNormalEquations *p1, *p2, *p3, *pempty;

      public:
        void setUp()
        { 
          p1 = new ImagingNormalEquations();
          p2 = new ImagingNormalEquations();
          p3 = new ImagingNormalEquations();
          pempty = new ImagingNormalEquations();
        }

        void tearDown()
        {
          delete p1;
          delete p2;
          delete p3;
          delete pempty;
        }

        void testConstructors()
        {
          Params ip;
          ip.add("Value0");
          ip.add("Value1");
          ip.add("Value2");
          delete p1;
          p1 = new ImagingNormalEquations(ip);
          CPPUNIT_ASSERT(p1->parameters().names().size()==3);
          CPPUNIT_ASSERT(p1->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p1->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p1->parameters().names()[2]=="Value2");
        }

        void testCopy()
        {
          Params ip;
          ip.add("Value0");
          ip.add("Value1");
          ip.add("Value2");
          delete p1;
          p1 = new ImagingNormalEquations(ip);
          delete p2;
          p2 = new ImagingNormalEquations(*p1);
          CPPUNIT_ASSERT(p2->parameters().names().size()==3);
          CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
        }

        
        void testBlobStream() {
          Params ip;
          ip.add("Value0");
          ip.add("Value1", 1.5);
          uint imsize=10*10;
          casa::Vector<double> im(imsize);
          im.set(3.0);
          ip.add("Image2", im);

	  delete p1;
          p1 = new ImagingNormalEquations(ip);
          LOFAR::BlobString b1(false);
          LOFAR::BlobOBufString bob(b1);
          LOFAR::BlobOStream bos(bob);
          bos << *p1;
          Params pnew;
          LOFAR::BlobIBufString bib(b1);
          LOFAR::BlobIStream bis(bib);
          p2 = new ImagingNormalEquations();
          bis >> *p2;
          //CPPUNIT_ASSERT(p2->parameters().names()[0]=="Image2");
          //CPPUNIT_ASSERT(p2->parameters().names().size()==3);
          //CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value0");
          //CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value1");
        }
    };

  }
}
