/// @file 
/// $brief Tests of the multi-chunk iterator adapter
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

#ifndef TIME_CHUNK_ITERATOR_ADAPTER_TEST_H
#define TIME_CHUNK_ITERATOR_ADAPTER_TEST_H

// boost includes
#include <boost/shared_ptr.hpp>

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>
// own includes
#include <dataaccess/TableDataSource.h>
#include <dataaccess/IConstDataSource.h>
#include <dataaccess/TimeChunkIteratorAdapter.h>
#include <askap/AskapError.h>
#include "TableTestRunner.h"
#include <askap/AskapUtil.h>


namespace askap {

namespace accessors {

class TimeChunkIteratorAdapterTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TimeChunkIteratorAdapterTest);
  CPPUNIT_TEST(testTimeChunks);  
  CPPUNIT_TEST_EXCEPTION(testReadOnlyBuffer,AskapError);  
  CPPUNIT_TEST_EXCEPTION(testReadOnlyAccessor,AskapError); 
  CPPUNIT_TEST_EXCEPTION(testNoResume,AskapError); 
  CPPUNIT_TEST_SUITE_END();
protected:
  static size_t countSteps(const IConstDataSharedIter &it) {
     size_t counter;
     for (counter = 0; it!=it.end(); ++it,++counter) {}
     return counter;     
  }
public:
  void testTimeChunks() {
     TableConstDataSource ds(TableTestRunner::msName());
     IDataConverterPtr conv=ds.createConverter();
     conv->setEpochFrame(); // ensures seconds since 0 MJD
     CPPUNIT_ASSERT_EQUAL(size_t(420), countSteps(ds.createConstIterator(conv)));
     boost::shared_ptr<TimeChunkIteratorAdapter> it(new TimeChunkIteratorAdapter(ds.createConstIterator(conv)));
     CPPUNIT_ASSERT_EQUAL(size_t(420), countSteps(it));
     it.reset(new TimeChunkIteratorAdapter(ds.createConstIterator(conv),599));
     size_t counter = 0;
     for (;it->moreDataAvailable();++counter) {
          CPPUNIT_ASSERT_EQUAL(size_t(1),countSteps(it));
          if (it->moreDataAvailable()) {
              it->resume();
          }
     }
     CPPUNIT_ASSERT_EQUAL(size_t(420), counter);     
     // now trying bigger chunks
     it.reset(new TimeChunkIteratorAdapter(ds.createConstIterator(conv),5990));
     for (counter = 0; it->moreDataAvailable(); ++counter) {
          CPPUNIT_ASSERT_EQUAL(size_t(10),countSteps(it));
          if (it->moreDataAvailable()) {
              it->resume();
          }
     }
     CPPUNIT_ASSERT_EQUAL(size_t(42), counter);     
     
  }
  
  void testReadOnlyBuffer() {
     TableConstDataSource ds(TableTestRunner::msName());
     boost::shared_ptr<TimeChunkIteratorAdapter> it(new TimeChunkIteratorAdapter(ds.createConstIterator()));
     // this should generate an exception
     it->buffer("TEST");
  }

  void testReadOnlyAccessor() {
     TableConstDataSource ds(TableTestRunner::msName());
     boost::shared_ptr<TimeChunkIteratorAdapter> it(new TimeChunkIteratorAdapter(ds.createConstIterator()));
     boost::shared_ptr<IDataAccessor> acc;
     try {
        boost::shared_ptr<IDataAccessor> staticAcc(it->operator->(),utility::NullDeleter());
        ASKAPASSERT(staticAcc);
        acc = staticAcc;
     }
     catch (const AskapError &) {
        // just to ensure no exception is thrown from the try-block
        CPPUNIT_ASSERT(false);
     }
     CPPUNIT_ASSERT(acc);
     // this should generate an exception
     acc->rwVisibility();
  }
  
  void testNoResume() {
     TableConstDataSource ds(TableTestRunner::msName());
     IDataConverterPtr conv=ds.createConverter();
     conv->setEpochFrame(); // ensures seconds since 0 MJD
     boost::shared_ptr<TimeChunkIteratorAdapter> it(new TimeChunkIteratorAdapter(ds.createConstIterator(conv),5990));     
     try {
        // this code shouldn't throw AskapError 
        CPPUNIT_ASSERT(it->hasMore());        
        boost::shared_ptr<IConstDataIterator> cit = boost::dynamic_pointer_cast<IConstDataIterator>(it);
        CPPUNIT_ASSERT(cit);
        cit->next();
        CPPUNIT_ASSERT(cit->hasMore());        
        // just to access some data field
        *(*cit);
        (*cit)->antenna1();
        // access those fields directly
        *(*it);
        (*it)->antenna1();
        CPPUNIT_ASSERT_EQUAL(size_t(9),countSteps(it));
        CPPUNIT_ASSERT(it->moreDataAvailable());
        CPPUNIT_ASSERT(!it->hasMore());
     }
     catch (const AskapError &) {
        CPPUNIT_ASSERT(false);
     }
     // the following should throw AskapError
     CPPUNIT_ASSERT(it->next());
  }
 
};

} // namespace accessors

} // namespace askap

#endif // #ifndef TIME_CHUNK_ITERATOR_ADAPTER_TEST_H


