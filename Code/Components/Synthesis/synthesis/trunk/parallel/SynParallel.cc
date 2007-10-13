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

#include <parallel/SynParallel.h>

#include <sstream>

using namespace std;
using namespace conrad;
using namespace conrad::cp;
using namespace conrad::scimath;

namespace conrad
{
	namespace synthesis
	{

		SynParallel::SynParallel(int argc, const char** argv)
		{
			// Initialize MPI (also succeeds if no MPI available).
			MPIConnection::initMPI (argc, argv);
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
				MWIos::setName (ostr.str());

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

			itsModel = Params::ShPtr(new Params());
			CONRADCHECK(itsModel, "Model not defined correctly");

		}

		SynParallel::~SynParallel()
		{
			if (isParallel())
			{
				std::cout << "Exiting MPI"<< std::endl;
				MPIConnection::endMPI();
			}
		}

		/// Is this running in parallel?
		bool SynParallel::isParallel()
		{
			return itsIsParallel;
		}

		/// Is this the master?
		bool SynParallel::isMaster()
		{
			return itsIsMaster;
		}

		/// Is this a worker?
		bool SynParallel::isWorker()
		{
			return itsIsWorker;
		}

		conrad::scimath::Params::ShPtr& SynParallel::params()
		{
			return itsModel;
		}

		std::ostream& SynParallel::os()
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
		void SynParallel::initConnections()
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

		// Send the model to all workers
		void SynParallel::broadcastModel()
		{
			if (isParallel()&&isMaster())
			{
				CONRADCHECK(itsModel, "Model not defined prior to broadcast")
				casa::Timer timer;
				timer.mark();

				LOFAR::BlobString bs;
				bs.resize(0);
				LOFAR::BlobOBufString bob(bs);
				LOFAR::BlobOStream out(bob);
				out.putStart("model", 1);
				out << *itsModel;
				out.putEnd();
				itsConnectionSet->writeAll(bs);
				os() << "Sent model to the workers via MPI in "<< timer.real()
				    << " seconds "<< std::endl;
			}
		}

		// Receive the model from the master
		void SynParallel::receiveModel()
		{
			if (isParallel()&&isWorker())
			{
				CONRADCHECK(itsModel, "Model not defined prior to receiving")
				casa::Timer timer;
				timer.mark();
				os() << "Receiving model from the master via MPI"<< std::endl;
				LOFAR::BlobString bs;
				bs.resize(0);
				itsConnectionSet->read(0, bs);
				LOFAR::BlobIBufString bib(bs);
				LOFAR::BlobIStream in(bib);
				int version=in.getStart("model");
				CONRADASSERT(version==1);
				in >> *itsModel;
				in.getEnd();
				os() << "Received model from the master via MPI in "<< timer.real()
				    << " seconds "<< std::endl;
			}
		}
		
		string SynParallel::substituteWorkerNumber(const string& s) 
		{
			casa::Regex reg("\%w");
			ostringstream oos;
			if(itsNNode>1) {
				oos << itsRank-1;
			}
			else {
				oos << 0;
			}
			casa::String cs(s);
			cs.gsub(reg, oos.str());
			return string(cs);
		}


	}
}
