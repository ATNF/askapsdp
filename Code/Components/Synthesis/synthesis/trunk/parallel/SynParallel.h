/// @file
///
/// SynParallel: Support for parallel applications
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_SYNPARALLEL_H_
#define CONRAD_SYNTHESIS_SYNPARALLEL_H_

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
		/// @details Provides generic methods for parallel algorithms
		/// @ingroup parallel
		class SynParallel
		{
			public:

				/// @brief Constructor from ParameterSet
				/// @param argc Number of command line inputs
				/// @param argv Command line inputs
				SynParallel(int argc, const char** argv);

				~SynParallel();

				/// @brief Return an output stream suitable for use in parallel environment
				std::ostream& os();

				/// Calculate the normalequations
				/// @params model Model from which the normal equations are to be derived
				virtual void calcNE(conrad::scimath::Params& model);

				/// Solve the normal equations
				/// @params model Model to be updated
				virtual void solveNE(conrad::scimath::Params& model);
				
      	/// Write the results
      	/// @params model Model to be written
      	virtual void writeModel(const conrad::scimath::Params& model);

			protected:
				/// Initialize the MPI connections
				void initConnections();

				/// @brief Send the normal equations
				void sendNE();

				/// @brief Receive the normal equations
				void receiveNE();

				/// @brief Broadcast the model
				/// @params model Model to be broadcast
				void broadcastModel(const conrad::scimath::Params& model);

				/// @brief Receive the model
				/// @params model Model from which the normal equations are to be derived
				void receiveModel(conrad::scimath::Params& model);

				conrad::cp::MPIConnectionSet::ShPtr itsConnectionSet;

				conrad::scimath::NormalEquations itsNe; // Normal equations
				conrad::scimath::Solver::ShPtr itsSolver; // Image solver to be used

				int itsRank; // Rank of process
				int itsNNode; // Number of nodes

				bool itsIsParallel; // Is this parallel? itsNNode > 1?
				bool itsIsSolver; // Is this the solver?

		};

	}
}
#endif
