/// @file
/// @brief Connection to workers based on MPI
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
/// @author Ger van Diepen <diepen@astron.nl>


//#include <lofar_config.h>

// System includes
#include <unistd.h>
#include <limits>

#include <mwcommon/MPIConnection.h>
#include <mwcommon/MWError.h>

#ifdef HAVE_MPI
# include <mpi.h>
#endif

#include <iostream>
using namespace std;

namespace askap { namespace mwbase {

  MPIConnection::MPIConnection (int destinationRank, int tag)
    : itsDestRank   (destinationRank),
      itsTag        (tag)
  {}

  MPIConnection::~MPIConnection()
  {}


#ifdef HAVE_MPI

  int MPIConnection::getMessageLength()
  {
    return -1;
  }

  void MPIConnection::receive(void* buf, size_t size)
  {
    const unsigned int c_maxint = std::numeric_limits<int>::max();
    ASKAPCHECK(buf != 0, "MPIConnection::receive() Null buf pointer passed");

    // First receive the size of the payload to be received,
    // remembering the size parameter passed to this function is
    // just the maximum size of the buffer, and hence the maximum
    // number of bytes that can be received.
    unsigned long payloadSize;
    int result = MPI_Recv(&payloadSize, 1, MPI_UNSIGNED_LONG,
            itsDestRank, itsTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (result != MPI_SUCCESS) {
        ASKAPTHROW (MWError, "MPIConnection::receive on rank " << getRank()
                << " failed: " << size << " bytes from rank " << itsDestRank
                << " using tag " << itsTag);
    }

    // Receive the smaller of size or payloadSize
    size_t remaining = (payloadSize > size) ? size : payloadSize;

    while (remaining > 0) {
        size_t offset = size - remaining;
        void* addr = addOffset(buf, offset);
        if (remaining >= c_maxint) {
            result = MPI_Recv(addr, c_maxint, MPI_BYTE,
                    itsDestRank, itsTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            remaining -= c_maxint;
        } else {
            result = MPI_Recv(addr, remaining, MPI_BYTE,
                    itsDestRank, itsTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            remaining = 0;
        }
        if (result != MPI_SUCCESS) {
            ASKAPTHROW (MWError, "MPIConnection::receive on rank " << getRank()
                    << " failed: " << size << " bytes from rank " << itsDestRank
                    << " using tag " << itsTag);
        }
    }

    ASKAPCHECK(remaining == 0, "MPIConnection::receive() Didn't receive all data");
  }

  void MPIConnection::send (const void* buf, size_t size)
  {
    const unsigned int c_maxint = std::numeric_limits<int>::max();
    ASKAPCHECK(buf != 0, "MPIConnection::send() Null buf pointer passed");

    // First send the size of the buffer.
    unsigned long lsize = size;  // Promote for simplicity
    int result = MPI_Send(&lsize, 1, MPI_UNSIGNED_LONG, itsDestRank, itsTag, MPI_COMM_WORLD);
    if (result != MPI_SUCCESS) {
        ASKAPTHROW (MWError, "MPIConnection::send on rank " << getRank()
                << " failed: " << size << " bytes to rank " << itsDestRank
                << " using tag " << itsTag);
    }

    // Send in chunks of size MAXINT until complete
    size_t remaining = size;
    while (remaining > 0) {
        size_t offset = size - remaining;

        void* addr = addOffset(buf, offset);
        if (remaining >= c_maxint) {
            result = MPI_Send(addr, c_maxint, MPI_BYTE,
                    itsDestRank, itsTag, MPI_COMM_WORLD);
            remaining -= c_maxint;
        } else {
            result = MPI_Send(addr, remaining, MPI_BYTE,
                    itsDestRank, itsTag, MPI_COMM_WORLD);
            remaining = 0;
        }
        if (result != MPI_SUCCESS) {
            ASKAPTHROW (MWError, "MPIConnection::send on rank " << getRank()
                    << " failed: " << size << " bytes to rank " << itsDestRank
                    << " using tag " << itsTag);
        }
    }

    ASKAPCHECK(remaining == 0, "MPIConnection::send() Didn't send all data");
  }

  bool MPIConnection::isConnected() const
  {
    return true;
  }

  void MPIConnection::initMPI (int argc, const char**& argv)
  {
    // Only initialize if not done yet.
    int initialized = 0;
    MPI_Initialized (&initialized);
    if (!initialized) {
      MPI_Init (&argc, &const_cast<char**&>(argv));
    }
  }

  void MPIConnection::endMPI()
  {
    // Only finalize if not done yet.
    int finalized = 0;
    MPI_Finalized (&finalized);
    if (!finalized) {
      MPI_Finalize();
    }
  }

  int MPIConnection::getRank()
  {
    int rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    return rank;
  }

  int MPIConnection::getNrNodes()
  {
    int size;
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    return size;
  }

  std::string MPIConnection::getNodeName()
  {
    char name[MPI_MAX_PROCESSOR_NAME];
    int resultlen;
    MPI_Get_processor_name(name, &resultlen);
    return std::string(name);
  }

#else


  int MPIConnection::getMessageLength()
  {
    ASKAPTHROW (MWError, "MPIConnection::getMessageLength cannot be used: "
		 "configured without MPI");
  }

  void MPIConnection::receive (void*, size_t)
  {
    ASKAPTHROW (MWError, "MPIConnection::receive cannot be used: "
		 "configured without MPI");
  }

  void MPIConnection::send (const void*, size_t)
  {
    ASKAPTHROW (MWError, "MPIConnection::send cannot be used: "
		 "configured without MPI");
  }

  bool MPIConnection::isConnected() const
  {
    return false;
  }

  void MPIConnection::initMPI (int, const char**&)
  {}

  void MPIConnection::endMPI()
  {}

  int MPIConnection::getRank()
  {
    return 0;
  }

  int MPIConnection::getNrNodes()
  {
    return 1;
  }

  std::string MPIConnection::getNodeName()
  {
    const int HOST_NAME_MAXLEN = 256;
    char name[HOST_NAME_MAXLEN];
    gethostname(name, HOST_NAME_MAXLEN);
    return std::string(name);
  }


#endif

  void* MPIConnection::addOffset(const void *ptr, size_t offset)
  {
      char *cptr = static_cast<char*>(const_cast<void*>(ptr));
      cptr += offset;

      return cptr;
  }


}} // end namespaces
