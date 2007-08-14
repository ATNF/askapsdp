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
				
				/// Initialize for calculations
				virtual void initialize();

				/// Finalize calculations
				virtual void finalize();

				/// Calculate the normalequations
				virtual void calcNE() = 0;

				/// Solve the normal equations
				virtual void solveNE() = 0;
				
      	/// Write the results
      	virtual void writeModel() = 0;
      	
      	/// Is this running in parallel?
      	bool isParallel() {return itsIsParallel;};

      	/// Is this the solver?
      	bool isSolver() {return itsIsSolver;};

      	/// Is this a prediffer?
      	bool isPrediffer() {return itsIsPrediffer;};
      	
      	/// Return the model
      	conrad::scimath::Params::ShPtr& params();

			protected:
				/// Initialize the MPI connections
				void initConnections();

				/// @brief Send the normal equations
				void sendNE();

				/// @brief Receive the normal equations
				void receiveNE();

				/// @brief Broadcast the model
				void broadcastModel();

				/// @brief Receive the model
				void receiveModel();

				conrad::cp::MPIConnectionSet::ShPtr itsConnectionSet;
				
				conrad::scimath::Params::ShPtr itsModel; // Model
				conrad::scimath::NormalEquations::ShPtr itsNe; // Normal equations
				conrad::scimath::Solver::ShPtr itsSolver; // Image solver to be used

				int itsRank; // Rank of process
				int itsNNode; // Number of nodes

				bool itsIsParallel; // Is this parallel? itsNNode > 1?
				bool itsIsSolver; // Is this the solver?
				bool itsIsPrediffer; // Is this a prediffer?

		};

	}
}
#endif
