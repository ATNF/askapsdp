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

#include <fitting/Params.h>
#include <utils/ChangeMonitor.h>

#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

#include <casa/Arrays/Matrix.h>

#include <askap/AskapError.h>

#include <cppunit/extensions/HelperMacros.h>

namespace askap
{
  namespace scimath
  {

    class ParamsTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(ParamsTest);
      CPPUNIT_TEST(testEmpty);
      CPPUNIT_TEST(testIndices);
      CPPUNIT_TEST(testAddition);
      CPPUNIT_TEST(testValues);
      CPPUNIT_TEST(testCongruent);
      CPPUNIT_TEST(testCompletions);
      CPPUNIT_TEST(testCopy);
      CPPUNIT_TEST(testArraySlice);
      CPPUNIT_TEST(testBlobStream);
      CPPUNIT_TEST_EXCEPTION(testDuplicate, askap::CheckError);
      CPPUNIT_TEST_EXCEPTION(testNotScalar, askap::CheckError);
      CPPUNIT_TEST(testChangeMonitor);
      CPPUNIT_TEST_SUITE_END();

      private:
        Params *p1, *p2, *p3, *pempty;

      public:
        void setUp()
        {
          p1 = new Params();
          p2 = new Params();
          p3 = new Params();
          pempty = new Params();
        }

        void tearDown()
        {
          delete p1;
          delete p2;
          delete p3;
          delete pempty;
        }

        void testEmpty()
        {
          CPPUNIT_ASSERT(p1->names().size()==0);
          CPPUNIT_ASSERT(p1->freeNames().size()==0);
        }
        void testDuplicate()
// Should throw invalid_argument
        {
          p1->add("Dup0");
          p1->add("Dup0");
        }

        void testNotScalar()
// Should throw invalid_argument
        {
          p1->add("NS0", casa::Vector<double>(100));
          double result=p1->scalarValue("NS0");
          p1->add("DoSomethingWithResult", result);
        }

        void testCompletions()
        {
          CPPUNIT_ASSERT( p1->size()==0);
          for (uint i=0;i<10;i++)
          {
            casa::String name;
            {
              std::ostringstream s;
              s<<"Root." << i;
              p1->add(s.str());
            }
            {
              std::ostringstream s;
              s<<i<<".Root";
              p1->add(s.str());
            }
          }
          CPPUNIT_ASSERT(p1->names().size()==20);
          CPPUNIT_ASSERT(p1->completions("Roo*9").size()==1);
          CPPUNIT_ASSERT(p1->completions("Root.*").size()==10);
          CPPUNIT_ASSERT(p1->completions("Nothing").size()==0);
        }

        void testCopy()
        {
          CPPUNIT_ASSERT( p1->size()==0);
          p1->add("Copy0");
          CPPUNIT_ASSERT(p1->has("Copy0"));
          CPPUNIT_ASSERT(p1->isScalar("Copy0"));
          p1->add("Copy1", 1.5);
          CPPUNIT_ASSERT(p1->scalarValue("Copy1")==1.5);
          CPPUNIT_ASSERT(p1);
          Params pnew(*p1);
          CPPUNIT_ASSERT(pnew.size()==2);
          CPPUNIT_ASSERT(pnew.has("Copy0"));
          CPPUNIT_ASSERT(pnew.has("Copy1"));
          CPPUNIT_ASSERT(pnew.scalarValue("Copy1")==1.5);
        }

        void testValues()
        {
          p1->add("Value0", 1.5);
          CPPUNIT_ASSERT(p1->has("Value0"));
          casa::Array<double> im(casa::IPosition(2,10,10));
          im.set(3.0);
          p1->add("Value1", im);
          CPPUNIT_ASSERT(p1->value("Value1")(casa::IPosition(2,5,5))==3.0);
          CPPUNIT_ASSERT(p1->has("Value1"));
          CPPUNIT_ASSERT(!p1->isScalar("Value1"));
          CPPUNIT_ASSERT(p1->value("Value1").nelements()==100);
          p1->value("Value1").set(4.0);
          CPPUNIT_ASSERT(p1->value("Value1")(casa::IPosition(2,5,5))==4.0);
        }
        
        void testArraySlice()
        {
          casa::Matrix<double> im(10,15,-1.0);
          for (casa::uInt row = 0; row<im.nrow(); ++row) {
               im.row(row).set(double(row));
          }
          p1->add("BigArray",im.shape());
          for (casa::uInt row = 0; row<im.nrow(); ++row) {
               casa::Vector<double> vec(im.ncolumn(),double(row));
               casa::Array<double> vecArr(vec.reform(casa::IPosition(2,1,vec.nelements())));
               casa::IPosition blc(2,int(row),0);
               p1->update("BigArray",vecArr, blc);
          }
          casa::Matrix<double> res(p1->value("BigArray"));
          for (casa::uInt x = 0; x<res.nrow(); ++x) {
               for (casa::uInt y = 0; y<res.ncolumn(); ++y) {
                    CPPUNIT_ASSERT(x<im.nrow());
                    CPPUNIT_ASSERT(y<im.ncolumn());
                    CPPUNIT_ASSERT(fabs(im(x,y)-res(x,y))<1e-7);                    
               }
          }
          
        }

        void testIndices()
        {
          CPPUNIT_ASSERT( p1->size()==0);
          p1->add("Ind0");
          CPPUNIT_ASSERT(p1->has("Ind0"));
          CPPUNIT_ASSERT(!p1->has("Ind1"));
          p1->add("Ind1");
          CPPUNIT_ASSERT(!pempty->has("Null"));
        }

        void testAddition()
        {
          CPPUNIT_ASSERT( p1->size()==0);
          p1->add("Add0");
          CPPUNIT_ASSERT( p1->size()==1);
          p1->add("Add1", 1.4);
          CPPUNIT_ASSERT( p1->scalarValue("Add1")==1.4);
          CPPUNIT_ASSERT( p1->size()==2);
          const ChangeMonitor cm = p1->monitorChanges("Add1");
          CPPUNIT_ASSERT(!p1->isChanged("Add1",cm));
          p1->update("Add1", 2.6);
          CPPUNIT_ASSERT( p1->scalarValue("Add1")==2.6);
          CPPUNIT_ASSERT(p1->isChanged("Add1",cm));
        }

        void testCongruent()
        {
          CPPUNIT_ASSERT( p1->size()==0);
          p1->add("foo");
          CPPUNIT_ASSERT( p1->size()==1);
          CPPUNIT_ASSERT( !(p1->isCongruent(*p2)));
          p2->add("bar");
          CPPUNIT_ASSERT( !(p1->isCongruent(*p2)));
          p3->add("foo");
          CPPUNIT_ASSERT( p1->isCongruent(*p3));
        }
        
        void testBlobStream() {
          p1->add("Copy0");
          p1->add("Copy1", 1.5);
          LOFAR::BlobString b1(false);
          LOFAR::BlobOBufString bob(b1);
          LOFAR::BlobOStream bos(bob);
          bos << *p1;
          Params pnew;
          LOFAR::BlobIBufString bib(b1);
          LOFAR::BlobIStream bis(bib);
          bis >> pnew;
          CPPUNIT_ASSERT(pnew.has("Copy0"));
          CPPUNIT_ASSERT(pnew.has("Copy1"));
          CPPUNIT_ASSERT(pnew.scalarValue("Copy1")==1.5);
          
        }
        
        void testChangeMonitor() {
          p1->add("Par1", 0.1);
          p1->add("Par2", casa::Vector<double>(5,1.));
          const ChangeMonitor cm1Par1 = p1->monitorChanges("Par1");
          const ChangeMonitor cm1Par2 = p1->monitorChanges("Par2");
          CPPUNIT_ASSERT(!p1->isChanged("Par1",cm1Par1));
          CPPUNIT_ASSERT(!p1->isChanged("Par2",cm1Par2));
          p1->update("Par1",-0.1);
          CPPUNIT_ASSERT(p1->isChanged("Par1",cm1Par1));
          CPPUNIT_ASSERT(!p1->isChanged("Par2",cm1Par2));
          p1->update("Par2", casa::Vector<double>(5,1.1));
          CPPUNIT_ASSERT(p1->isChanged("Par1",cm1Par1));
          CPPUNIT_ASSERT(p1->isChanged("Par2",cm1Par2));
          // second level of change monitoring
          const ChangeMonitor cm2Par1 = p1->monitorChanges("Par1");
          const ChangeMonitor cm2Par2 = p1->monitorChanges("Par2");
          CPPUNIT_ASSERT(!p1->isChanged("Par1",cm2Par1));
          CPPUNIT_ASSERT(!p1->isChanged("Par2",cm2Par2));
          for (int i=0;i<20; ++i) {
               p1->update("Par1",-0.1+double(i));
               p1->update("Par2", casa::Vector<double>(5,1.1+double(i)));               
          }
          CPPUNIT_ASSERT(p1->isChanged("Par1",cm2Par1));
          CPPUNIT_ASSERT(p1->isChanged("Par2",cm2Par2));
          CPPUNIT_ASSERT(p1->isChanged("Par1",cm1Par1));
          CPPUNIT_ASSERT(p1->isChanged("Par2",cm1Par2));
        }
    };

  }
}
