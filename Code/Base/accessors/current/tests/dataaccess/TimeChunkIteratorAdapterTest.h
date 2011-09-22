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
#include "TableTestRunner.h"

namespace askap {

namespace accessors {

class TimeChunkIteratorAdapterTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TimeChunkIteratorAdapterTest);
  CPPUNIT_TEST(testAdapter);  
  CPPUNIT_TEST_SUITE_END();
protected:
  static size_t countSteps(const IConstDataSharedIter &it) {
     size_t counter;
     for (counter = 0; it!=it.end(); ++it,++counter) {}
     return counter;     
  }
public:
  void testAdapter() {
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
  }
};

} // namespace accessors

} // namespace askap

#endif // #ifndef TIME_CHUNK_ITERATOR_ADAPTER_TEST_H


