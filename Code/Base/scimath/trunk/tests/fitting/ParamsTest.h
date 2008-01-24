#include <fitting/Params.h>

#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>


#include <conrad/ConradError.h>

#include <cppunit/extensions/HelperMacros.h>

namespace conrad
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
      CPPUNIT_TEST(testBlobStream);
      CPPUNIT_TEST_EXCEPTION(testDuplicate, conrad::CheckError);
      CPPUNIT_TEST_EXCEPTION(testNotScalar, conrad::CheckError);
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
          p1->update("Add1", 2.6);
          CPPUNIT_ASSERT( p1->scalarValue("Add1")==2.6);
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
    };

  }
}
