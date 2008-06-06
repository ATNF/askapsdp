/// @file DataAccessTestImpl.h
///
/// DataAccessTestImpl: Implementation of the Data Access test class
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
