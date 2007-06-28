//# MPIConnection.cc: Connection to workers based on MPI
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

//#include <lofar_config.h>

#include <mwcommon/MPIConnection.h>
#include <mwcommon/MWError.h>

#ifdef HAVE_MPI
# include <mpi.h>
#endif

#include <iostream>
using namespace std;

namespace conrad { namespace cp {

  MPIConnection::MPIConnection (int destinationRank, int tag)
    : itsDestRank   (destinationRank),
      itsTag        (tag)
  {}

  MPIConnection::~MPIConnection()
  {}


#ifdef HAVE_MPI

  int MPIConnection::getMessageLength()
  {
    int result = MPI_SUCCESS;
    MPI_Status status;
    result = MPI_Probe (itsDestRank, itsTag, MPI_COMM_WORLD, &status);
    int size;
    MPI_Get_count (&status, MPI_BYTE, &size);
    return size;
  }

  void MPIConnection::receive (void* buf, unsigned size)
  {
    //cout << "MPI receive " << size << " bytes on rank " << getRank()
    //     << " from rank " << itsDestRank << ", tag " << itsTag << endl;
    int result = MPI_SUCCESS;
    MPI_Status status;
    result = MPI_Recv (buf, size, MPI_BYTE,
		       itsDestRank, itsTag, MPI_COMM_WORLD, &status);
    if (result != MPI_SUCCESS) {
      CONRADTHROW (MWError, "MPIConnection::receive on rank " << getRank()
		   << " failed: " << size << " bytes from rank " << itsDestRank
		   << " using tag " << itsTag);
    }
  }

  void MPIConnection::send (const void* buf, unsigned size)
  {
    //cout << "MPI send " << size << " bytes on rank " << getRank()
    //     << " to rank " << itsDestRank << ", tag " << itsTag << endl;
    int result = MPI_SUCCESS;
    result = MPI_Send (const_cast<void*>(buf), size, MPI_BYTE,
		       itsDestRank, itsTag, MPI_COMM_WORLD);
    if (result != MPI_SUCCESS) {
      CONRADTHROW (MWError, "MPIConnection::send on rank " << getRank()
		   << " failed: " << size << " bytes to rank " << itsDestRank
		   << " using tag " << itsTag);
    }
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


#else


  int MPIConnection::getMessageLength()
  {
    CONRADTHROW (MWError, "MPIConnection::getMessageLength cannot be used: "
		 "configured without MPI");
  }

  void MPIConnection::receive (void*, unsigned)
  {
    CONRADTHROW (MWError, "MPIConnection::receive cannot be used: "
		 "configured without MPI");
  }

  void MPIConnection::send (const void*, unsigned)
  {
    CONRADTHROW (MWError, "MPIConnection::send cannot be used: "
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


#endif


}} // end namespaces
