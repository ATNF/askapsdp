/// @file DataAccessTestImpl.h
///
/// DataAccessTestImpl: Implementation of the Data Access test class
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 
#ifndef I_DATA_ACCESS_TEST_IMPL_H
#define I_DATA_ACCESS_TEST_IMPL_H

#include <boost/shared_ptr.hpp>
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IFlagDataAccessor.h>
#include <dataaccess/IDataSource.h>
#include <dataaccess/SharedIter.h>


namespace askap {

namespace synthesis {

struct DataAccessTestImpl {
   /// demonstration of flagging from the given iterator position until the
   /// end of the block pointed by the iterator
   static void flaggingRoutine(const IDataSharedIter &di);

   /// demonstration of the read-only access
   static void readOnlyRoutine(const IConstDataSharedIter &cdi);

   /// obtaining iterators, invoke other methods
   static void doTheJob(const boost::shared_ptr<IDataSource> &ds);
};

} // namespace synthesis
} // namespace askap

#endif // #ifndef I_DATA_ACCESS_TEST_IMPL_H
