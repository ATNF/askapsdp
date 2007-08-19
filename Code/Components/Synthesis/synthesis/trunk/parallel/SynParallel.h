/// @file
///
/// Provides generic methods for parallel algorithms using the measurement equation
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_SYNPARALLEL_H_
#define CONRAD_SYNTHESIS_SYNPARALLEL_H_

#include <fitting/Equation.h>
#include <fitting/Solver.h>
#include <fitting/NormalEquations.h>
#include <fitting/Params.h>

#include <mwcommon/MPIConnectionSet.h>

namespace conrad
{
	namespace synthesis
	{
		/// @brief Support for parallel algorithms 
		///
		/// @details Support for parallel applications in the area.
		/// An application is derived from this abstract base. The model used is that the
		/// application has many workers and one master, running in separate MPI processes
		/// or in one single thread. The master is the master so the number of processes
		/// is one more than the number of workers. 
		///
		/// If the number of nodes is 1 then everything occurs in the same process with
		/// no overall for transmission of model.
		///
		/// @ingroup parallel
		class SynParallel
		{
			public:

				/// @brief Constructor 
				/// @details The command line inputs are needed solely for MPI - currently no
				/// application specific information is passed on the command line.
				/// @param argc Number of command line inputs
				/// @param argv Command line inputs
				SynParallel(int argc, const char** argv);

				~SynParallel();

				/// @brief Return an output stream suitable for use in parallel environment
				/// @details Sending messages to std::cout can be error prone in a parallel
				/// environment. Hence one should write to this stream, which is currently
				/// connected to a file tagged with the rank number. Eventually the conrad
				/// logging system will be used.
				std::ostream& os();

				/// Write the model (runs only in the master)
				virtual void writeModel() = 0;

				/// Is this running in parallel?
				bool isParallel();

				/// Is this the master?
				bool isMaster();

				/// Is this a worker?
				bool isWorker();

				/// Return the model
				conrad::scimath::Params::ShPtr& params();

				/// @brief Broadcast the model to all workers
				void broadcastModel();

				/// @brief Receive the model from the master
				void receiveModel();

			protected:
				/// Initialize the MPI connections
				void initConnections();

				/// The set of all connections between processes. For the master, there
				/// are connections to every worker, but each worker has only one
				/// connection, which is to the master.
				conrad::cp::MPIConnectionSet::ShPtr itsConnectionSet;

				/// The model
				conrad::scimath::Params::ShPtr itsModel;

				/// Rank of this process : 0 for the master, >0 for workers
				int itsRank;

				/// Number of nodes
				int itsNNode;

				/// Is this parallel? itsNNode > 1?
				bool itsIsParallel;

				/// Is this the Master?
				bool itsIsMaster;

				/// Is this a worker?
				bool itsIsWorker;

		};

	}
}
#endif
