/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 
#include <mwcommon/MPIConnection.h>
#include <mwcommon/MPIConnectionSet.h>
#include <mwcommon/MWIos.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <casa/OS/Timer.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>

#include <askap_askapparallel.h>

#include <askap/AskapLogging.h>

#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <sstream>

using namespace std;
using namespace askap;
using namespace askap::cp;

namespace askap
{
  namespace cp
  {

    ASKAP_LOGGER(logger, ".askapparallel");

    AskapParallel::AskapParallel(int argc, const char** argv)
    {
      // Initialize MPI (also succeeds if no MPI available).
      MPIConnection::initMPI(argc, argv);
      itsNNode = MPIConnection::getNrNodes();
      itsRank = MPIConnection::getRank();

      itsIsParallel=(itsNNode>1);
      itsIsMaster=(itsRank==0);
      itsIsWorker=(!itsIsParallel)||(itsRank>0);

      initConnections();

      if (isParallel())
      {
        if (isMaster())
        {
          ASKAPLOG_INFO_STR(logger, "ASKAP program (parallel) running on "<< itsNNode
                              << " nodes (master/master)");
        }
        else
        {
          ASKAPLOG_INFO_STR(logger, "ASKAP program (parallel) running on "<< itsNNode
                              << " nodes (worker "<< itsRank << ")");
        }
      }
      else
      {
        ASKAPLOG_INFO_STR(logger, "ASKAP program (serial)");
      }

      ASKAPLOG_INFO_STR(logger, ASKAP_PACKAGE_VERSION);

    }

    AskapParallel::~AskapParallel()
    {
      if (isParallel())
      {
        ASKAPLOG_INFO_STR(logger, "Exiting MPI");
        MPIConnection::endMPI();
      }
    }

    /// Is this running in parallel?
    bool AskapParallel::isParallel()
    {
      return itsIsParallel;
    }

    /// Is this the master?
    bool AskapParallel::isMaster()
    {
      return itsIsMaster;
    }

    /// Is this a worker?
    bool AskapParallel::isWorker()
    {
      return itsIsWorker;
    }

    // Initialize connections
    void AskapParallel::initConnections()
    {
      if (isParallel())
      {
        itsConnectionSet=MPIConnectionSet::ShPtr(new MPIConnectionSet());
        if (isMaster())
        {
          // I am the master - I need a connection to every worker
          for (int i=1; i<itsNNode; ++i)
          {
            itsConnectionSet->addConnection(i, 0);
          }
        }
        if (isWorker())
        {
          // I am a worker - I only need to talk to the master
          itsConnectionSet->addConnection(0, 0);
        }
      }
    }

    string AskapParallel::substitute(const string& s)
    {
      casa::String cs(s);
      {
	casa::Regex regWork("\%w");
	ostringstream oos;
	if (itsNNode>1)
	  {
	    oos << itsRank-1;
	  }
	else
	  {
	    oos << 0;
	  }
	cs.gsub(regWork, oos.str());
      }
      casa::Regex regNode("\%n");
      {
	ostringstream oos;
	if (itsNNode>1)
	  {
	    oos << itsNNode-1;
	  }
	else
	  {
	    oos << 1;
	  }
	cs.gsub(regNode, oos.str());
      }
      return string(cs);
    }

  }
}
