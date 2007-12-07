/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
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

#include <conrad/ConradError.h>

#include <conradparallel/ConradParallel.h>

#include <sstream>

using namespace std;
using namespace conrad;
using namespace conrad::cp;

namespace conrad
{
  namespace cp
  {

    ConradParallel::ConradParallel(int argc, const char** argv)
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
        // For MPI, send all output to separate log files. Eventually we'll do this
        // using the log system.
        std::ostringstream ostr;
        casa::Path progname(argv[0]);
        ostr << progname.baseName() << "_tmp.cout" << itsRank;
        MWIos::setName(ostr.str());

        if (isMaster())
        {
          os() << "CONRAD program (parallel) running on "<< itsNNode
              << " nodes (master/master)"<< std::endl;
        }
        else
        {
          os() << "CONRAD program (parallel) running on "<< itsNNode
              << " nodes (worker "<< itsRank << ")"<< std::endl;
        }
      }
      else
      {
        os() << "CONRAD program (serial)"<< std::endl;
      }

    }

    ConradParallel::~ConradParallel()
    {
      if (isParallel())
      {
        os() << "Exiting MPI"<< std::endl;
        MPIConnection::endMPI();
      }
    }

    /// Is this running in parallel?
    bool ConradParallel::isParallel()
    {
      return itsIsParallel;
    }

    /// Is this the master?
    bool ConradParallel::isMaster()
    {
      return itsIsMaster;
    }

    /// Is this a worker?
    bool ConradParallel::isWorker()
    {
      return itsIsWorker;
    }

    std::ostream& ConradParallel::os()
    {
      if (isParallel())
      {
        if (isWorker())
        {
          MWCOUT << "WORKER " << itsRank << " : ";
          return MWCOUT;
        }
        else
        {
          std::cout << "MASTER : ";
          return std::cout;
        }
      }
      else
      {
        return std::cout;
      }
    }

    // Initialize connections
    void ConradParallel::initConnections()
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

    string ConradParallel::substitute(const string& s)
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
	    oos << 0;
	  }
	cs.gsub(regNode, oos.str());
      }
      return string(cs);
    }

  }
}
