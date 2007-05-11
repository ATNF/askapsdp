/// @file DataAccessTestImpl
///
/// DataAccessTest: Test of the Data Access layer. Due to the lack of
///                 real implementation all tests are actually compilation
///                 tests. The methods using the interface are gathered in
///                 the separate class DataAccessTestImpl. They will be
///                 called from here when the DataSource implementation
///                 is ready.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 
#ifndef I_DATA_ACCESS_TEST_H
#define I_DATA_ACCESS_TEST_H

#include <stdexcept>
#include "DataAccessTestImpl.h"

#include <cppunit/extensions/HelperMacros.h>

namespace conrad {
namespace synthesis {

class DataAccessTest : public CppUnit::TestFixture,
                       protected DataAccessTestImpl
{
    CPPUNIT_TEST_SUITE(DataAccessTest);
    CPPUNIT_TEST(testAccess);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp()
    {
      // do nothing at this stage as we only test compilation
    }
    void tearDown()
    {
      // do nothing at this stage as we only test compilation
    }
    void testAccess()
    {
      // do nothing at this stage as we only test compilation
    }
};
 
} // namespace synthesis
} // namespace conrad

#endif // #ifndef I_DATA_ACCESS_TEST_H
