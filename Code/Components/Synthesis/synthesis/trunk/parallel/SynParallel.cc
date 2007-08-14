/// @file
///
/// @brief Base class for parallel synthesis applications
/// @details
/// Supports parallel algorithms by providing methods for initialization
/// of MPI connections, sending normal equations and models around.
/// There is assumed to be one solver and many prediffers.
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

#include <conrad/ConradError.h>

#include <parallel/SynParallel.h>

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
			itsIsSolver=(itsRank==0);
			itsIsPrediffer=(!itsIsParallel)||(itsRank>0);

			initConnections();

			if (isParallel())
			{
				// For MPI, send all output to separate log files. Eventually we'll do this
				// using the log system.
				std::ostringstream ostr;
				ostr << itsRank;
				MWIos::setName ("cimager_tmp.cout" + ostr.str());

				if (isSolver())
				{
					os() << "CONRAD synthesis program (parallel) running on "<< itsNNode
					    << " nodes (master/solver)"<< std::endl;
				}
				else
				{
					os() << "CONRAD synthesis program (parallel) running on "<< itsNNode
					    << " nodes (prediffer "<< itsRank << ")"<< std::endl;
				}
			}
			else
			{
				os() << "CONRAD synthesis program (serial)"<< std::endl;
			}

			itsModel = Params::ShPtr(new Params());
			itsSolver = Solver::ShPtr(new Solver(*itsModel));
			itsNe = NormalEquations::ShPtr(new NormalEquations(*itsModel));

		}

		SynParallel::~SynParallel()
		{
			if (isParallel())
			{
				os() << "Exiting MPI"<< std::endl;
				MPIConnection::endMPI();
			}
		}

		conrad::scimath::Params::ShPtr& SynParallel::params()
		{
			return itsModel;
		}

		std::ostream& SynParallel::os()
		{
			if (isParallel())
			{
				if (isPrediffer())
				{
					MWCOUT << "PREDIFFER " << itsRank << " : ";
					return MWCOUT;
				}
				else
				{
					std::cout << "SOLVER : ";
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
				if (isSolver())
				{
					// I am the master - I need a connection to every worker
					for (int i=1; i<itsNNode; ++i)
					{
						itsConnectionSet->addConnection(i, 0);
					}
				}
				else
				{
					// I am a worker - I only need to talk to the master
					itsConnectionSet->addConnection(0, 0);
				}
			}
		}

		// Send the normal equations from this worker to the master as a blob
		void SynParallel::sendNE()
		{
			if (isParallel()&&isPrediffer())
			{
				casa::Timer timer;
				timer.mark();

				LOFAR::BlobString bs;
				bs.resize(0);
				LOFAR::BlobOBufString bob(bs);
				LOFAR::BlobOStream out(bob);
				out.putStart("ne", 1);
				out << itsRank << *itsNe;
				out.putEnd();
				itsConnectionSet->write(0, bs);
				os() << "Sent normal equations to the solver via MPI in "
				    << timer.real()<< " seconds "<< std::endl;
			}
		}

		// Receive the normal equations as a blob
		void SynParallel::receiveNE()
		{
			CONRADCHECK(itsSolver, "Solver not yet defined");
			if (isParallel()&&isSolver())
			{
				os() << "Waiting for normal equations"<< std::endl;
				casa::Timer timer;
				timer.mark();

				LOFAR::BlobString bs;
				int rank;
				for (int i=0; i<itsNNode-1; i++)
				{
					itsConnectionSet->read(i, bs);
					LOFAR::BlobIBufString bib(bs);
					LOFAR::BlobIStream in(bib);
					int version=in.getStart("ne");
					CONRADASSERT(version==1);
					in >> rank >> *itsNe;
					in.getEnd();
					itsSolver->addNormalEquations(*itsNe);
					os() << "Received normal equations from prediffer "<< rank
					    << " after "<< timer.real() << " seconds"<< std::endl;
				}
				os()<< "Received normal equations from all prediffers via MPI in "
				    << timer.real() << " seconds"<< std::endl;
			}
		}

		// Send the model to all workers
		void SynParallel::broadcastModel()
		{
			if (isParallel()&&isSolver())
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
				for (int i=1; i<itsNNode; ++i)
				{
					itsConnectionSet->write(i-1, bs);
				}
				os() << "Sent model to the prediffers via MPI in "<< timer.real()
				    << " seconds "<< std::endl;
			}
		}

		// Receive the model from the solver
		void SynParallel::receiveModel()
		{
			if (isParallel()&&isPrediffer())
			{
				CONRADCHECK(itsModel, "Model not defined prior to receiving")
				casa::Timer timer;
				timer.mark();
				LOFAR::BlobString bs;
				bs.resize(0);
				itsConnectionSet->read(0, bs);
				LOFAR::BlobIBufString bib(bs);
				LOFAR::BlobIStream in(bib);
				int version=in.getStart("model");
				CONRADASSERT(version==1);
				in >> *itsModel;
				in.getEnd();
				os() << "Received model from the solver via MPI in "<< timer.real()
				    << " seconds "<< std::endl;
			}
		}

		void SynParallel::writeModel()
		{
		}

	}
}