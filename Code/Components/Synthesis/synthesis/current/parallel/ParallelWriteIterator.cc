/// @file 
/// @brief iterator implementing parallel write
/// @details This is an implementation of data iterator
/// (see Base/accessors) which runs in a particular worker to
/// allow parallel writing of visibilities. Read operation is not
/// supported for simplicity. The server has to be executed at the
/// master side at the same time. It gathers the data (and distributes jobs
/// between workers). The decision was made to have this class in synthesis/parallel
/// rather than Base/accessors because it uses master-working specific code and
/// is not a general purpose class. The master (server iterator) is implemented as 
/// a static method of this class, so the communication protocol is encapsulated here.
///
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

#include <parallel/ParallelWriteIterator.h>
#include <parallel/ParallelIteratorStatus.h>
#include <askap_synthesis.h>
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

ASKAP_LOGGER(logger, ".parallel");

namespace askap {

namespace synthesis {

/// @brief constructor
/// @details 
/// @param comms communication object
ParallelWriteIterator::ParallelWriteIterator(askap::mwcommon::AskapParallel& comms) : 
   itsComms(comms), itsNotAtOrigin(false), itsAccessorValid(false)
{
  advance();
}    
    
// Return the data accessor (current chunk) in various ways	

/// @brief reference to data accessor (current chunk)
/// @return a reference to the current chunk
/// @note constness of the return type is changed to allow read/write
/// operations.
accessors::IDataAccessor& ParallelWriteIterator::operator*() const
{
  ASKAPCHECK(itsAccessorValid, "An attempt to obtain accessor following the end of iteration");
  return itsAccessor;
}
		
/// Switch the output of operator* and operator-> to one of 
/// the buffers. This is meant to be done to provide the same 
/// interface for a buffer access as exists for the original 
/// visibilities (e.g. it->visibility() to get the cube).
/// It can be used for an easy substitution of the original 
/// visibilities to ones stored in a buffer, when the iterator is
/// passed as a parameter to mathematical algorithms. 
/// 
/// The operator* and operator-> will refer to the chosen buffer
/// until a new buffer is selected or the chooseOriginal() method
/// is executed to revert operators to their default meaning
/// (to refer to the primary visibility data).
///
/// @param[in] bufferID  the name of the buffer to choose
///
void ParallelWriteIterator::chooseBuffer(const std::string &bufferID)
{
  ASKAPTHROW(AskapError, "An attempt to choose the buffer "<<bufferID<<
             ". Operation is not supported by the parallel iterator");
}

/// Switch the output of operator* and operator-> to the original
/// state (present after the iterator is just constructed) 
/// where they point to the primary visibility data. This method
/// is indended to cancel the results of chooseBuffer(casa::uInt)
///
void ParallelWriteIterator::chooseOriginal() {}

/// return any associated buffer for read/write access. The 
/// buffer is identified by its bufferID. The method 
/// ignores a chooseBuffer/chooseOriginal setting.
/// 
/// @param[in] bufferID the name of the buffer requested
/// @return a reference to writable data accessor to the
///         buffer requested
///
/// Because IDataAccessor has both const and non-const visibility()
/// methods defined separately, it is possible to detect when a
/// write operation took place and implement a delayed writing
accessors::IDataAccessor& ParallelWriteIterator::buffer(const std::string &bufferID) const
{
  ASKAPTHROW(AskapError, "An attempt to access the buffer "<<bufferID<<
             ". Operation is not supported by the parallel iterator");  
}
	
/// Restart the iteration from the beginning
void ParallelWriteIterator::init()
{
  ASKAPCHECK(!itsNotAtOrigin, "Restart of the iteration is not supported by the parallel iterator");
}
	
/// Checks whether there are more data available.
/// @return True if there are more data available
casa::Bool ParallelWriteIterator::hasMore() const throw()
{
  return itsAccessorValid;
}
	
/// advance the iterator one step further
/// @return True if there are more data (so constructions like
///         while(it.next()) {} are possible)
casa::Bool ParallelWriteIterator::next()
{
  itsNotAtOrigin = true;
  advance();
  return hasMore();
}

/// @brief obtain metadata for the next iteration
/// @details This is a core method of the class. It receives the
/// status message from the master and reads the metadata if not at
/// the last iteration. If not at the first iteration, it also syncronises
/// the visibility cube with the master before advancing to the next iteration.
void ParallelWriteIterator::advance()
{
  ASKAPDEBUGASSERT(itsComms.isWorker());
  if (itsNotAtOrigin) {
      // sync the result
  }
  // get status
  // update itsAccessorValid from status
  {
    LOFAR::BlobString bs;
    bs.resize(0);
    itsComms.connectionSet()->broadcast(bs,0);
    LOFAR::BlobIBufString bib(bs);
    LOFAR::BlobIStream in(bib);
    ParallelIteratorStatus status;
    in>>status;
    itsAccessorValid = status.itsHasMore;
  }
        
  if (itsAccessorValid) {
      // receive metadata, fill itsAccessor
  }
}

/// @brief server method
/// @details It iterates through the given iterator, serves metadata
/// to client iterators and combines visibilities in a single cube.
/// @param comms communication object
/// @param iter shared iterator to use
void ParallelWriteIterator::masterIteration(askap::mwcommon::AskapParallel& comms, const accessors::IDataSharedIter &iter)
{
  ASKAPDEBUGASSERT(comms.isMaster());
  accessors::IDataSharedIter it(iter);
  do {
    ParallelIteratorStatus status;
    status.itsHasMore = it.hasMore();
    if (status.itsHasMore) {
       status.itsNRow = it->nRow();
       status.itsNChan = it->nChannel(); // need to reduce to the actual number of channels
       status.itsNPol = it->nPol();
    }
    {
      LOFAR::BlobString bs;
      bs.resize(0);
      LOFAR::BlobOBufString bob(bs);
      LOFAR::BlobOStream out(bob);
      out << status;
      comms.connectionSet()->broadcast(bs,0);
    }
    
    if (it.hasMore()) {
        // send metadata
        // receive the result and store it in rwVisibility
    }
  } while (it.next());  
}


} // namespace synthesis

} // namespace askap
