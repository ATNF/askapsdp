#include <fitting/Params.h>
#include <fitting/ParamsCasaTable.h>

#include <tables/Tables/TableError.h>

#include <conrad/ConradError.h>

#include <cppunit/extensions/HelperMacros.h>

namespace conrad
{
  namespace scimath
  {

    class ParamsTableTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(ParamsTableTest);
      CPPUNIT_TEST(testCreate);
      CPPUNIT_TEST(testGet);
      CPPUNIT_TEST_SUITE_END();

      private:
        Params *p1;

      public:
        void setUp()
        {
          p1 = new Params();
          p1->add("par0", 10.0);
          p1->add("par1", 11.0);
          p1->add("par2", 12.0);
          p1->add("par3", 13.0);
          Axes axes1;
          axes1.add("Freq", 1e9, 2e9);
          casa::Array<double> arr1(casa::IPosition(1,10));
          arr1.set(99.0);
          p1->add("par4", arr1, axes1);
          Axes axes2;
          axes2.add("RA", -1.0, 1.0);
          axes2.add("DEC", -0.3, 0.5);
          casa::Array<double> arr2(casa::IPosition(2,10,10));
          arr2.set(137.1);
          p1->add("par5", arr2, axes2);
        }

        void tearDown()
        {
          delete p1;
        }

        void testCreate()
        {
          ParamsCasaTable pt("ParamsTableTest.tab", false);
          pt.setParameters(*p1);
        }

        void testGet()
        {
          ParamsCasaTable pt("ParamsTableTest.tab", true);
          Params ip;
          pt.getParameters(ip);
          CPPUNIT_ASSERT(p1->isCongruent(ip));
        }
    };

  }
}
