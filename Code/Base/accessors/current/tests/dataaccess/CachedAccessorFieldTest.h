/// @file
/// @brief Tests of the CachedAccessorField template
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
/// 

#ifndef CACHED_ACCESSOR_FIELD_TEST_H
#define CACHED_ACCESSOR_FIELD_TEST_H

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>

// own includes
#include <dataaccess/CachedAccessorField.h>

// std includes
#include <string>

namespace askap {

namespace accessors {

class CachedAccessorFieldTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CachedAccessorFieldTest);
  CPPUNIT_TEST(readOnDemandTest);
  CPPUNIT_TEST(writeTest);
  CPPUNIT_TEST_EXCEPTION(readRequiredTest, AskapError);
  CPPUNIT_TEST_EXCEPTION(readRequiredBeforeWriteTest, AskapError);
  CPPUNIT_TEST_EXCEPTION(readUnsyncedTest, AskapError);
  CPPUNIT_TEST_SUITE_END();
private:
  CachedAccessorField<std::string> itsCAF;
public:
  void stringFiller(std::string &str) const {
        str="filled by stringFiller";
  }
  void operator()(std::string &str) const {
       str="filled by operator()";
  }
  
  void readOnDemandTest() {
     CPPUNIT_ASSERT(!itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());
     std::string result = itsCAF.value(*this, &CachedAccessorFieldTest::stringFiller);
     CPPUNIT_ASSERT_EQUAL(std::string("filled by stringFiller"), result);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());     
     // no change expected as the value is cached
     result = itsCAF.value(*this);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());     
     CPPUNIT_ASSERT_EQUAL(std::string("filled by stringFiller"), result);
     // now invalidate and the update should become effective
     itsCAF.invalidate();
     CPPUNIT_ASSERT(!itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());
     result = itsCAF.value(*this);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());     
     CPPUNIT_ASSERT_EQUAL(std::string("filled by operator()"), result);
     // new the new value should be locked in
     result = itsCAF.value(*this, &CachedAccessorFieldTest::stringFiller);
     CPPUNIT_ASSERT_EQUAL(std::string("filled by operator()"), result);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());          
     // read operation is not needed now, can use method without parameters
     result = itsCAF.value();
     CPPUNIT_ASSERT_EQUAL(std::string("filled by operator()"), result);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());          
  }
  
  void writeTest() {
     std::string &ref = itsCAF.rwValue(*this, &CachedAccessorFieldTest::stringFiller);
     CPPUNIT_ASSERT_EQUAL(std::string("filled by stringFiller"), ref);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(itsCAF.flushNeeded());     
     ref = "overwritten";
     std::string result = itsCAF.value(*this, &CachedAccessorFieldTest::stringFiller);
     CPPUNIT_ASSERT_EQUAL(std::string("overwritten"), result);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(itsCAF.flushNeeded());  
     // can do as many writes as we like now
     itsCAF.rwValue(*this);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(itsCAF.flushNeeded());  
     itsCAF.rwValue();
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(itsCAF.flushNeeded());  
     // check the content of the cache
     result = itsCAF.value(*this);
     CPPUNIT_ASSERT_EQUAL(std::string("overwritten"), result);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(itsCAF.flushNeeded());     
     result = itsCAF.value();
     CPPUNIT_ASSERT_EQUAL(std::string("overwritten"), result);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(itsCAF.flushNeeded());     
     // now pretend to sync the cache
     itsCAF.flushed();
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());     
     // but the result is still the same
     result = itsCAF.value(*this);
     CPPUNIT_ASSERT_EQUAL(std::string("overwritten"), result);               
  }
  
  void readRequiredTest() {
     CPPUNIT_ASSERT(!itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());
     // the following would cause exception because reading is required
     itsCAF.value();
  }
  
  void readRequiredBeforeWriteTest() {
     CPPUNIT_ASSERT(!itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());
     // the following would cause exception because reading is required
     itsCAF.rwValue();
  }    
  
  void readUnsyncedTest() {
     CPPUNIT_ASSERT(!itsCAF.isValid());
     CPPUNIT_ASSERT(!itsCAF.flushNeeded());

     itsCAF.rwValue(*this);
     CPPUNIT_ASSERT(itsCAF.isValid());
     CPPUNIT_ASSERT(itsCAF.flushNeeded());  
     
     itsCAF.invalidate();
     // now we get exception if we attempted to read because sync is not done
     itsCAF.value(*this);     
  }
  
}; // class CachedAccessorFieldTest

} // namespace accessors

} // namespace askap

#endif // #ifndef CACHED_ACCESSOR_FIELD_TEST_H

