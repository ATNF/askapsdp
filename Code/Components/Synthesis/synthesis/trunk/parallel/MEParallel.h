/// @file
///
/// Provides generic methods for parallel algorithms using the measurement equation
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_MEPARALLEL_H_
#define CONRAD_SYNTHESIS_MEPARALLEL_H_

#include <parallel/SynParallel.h>

namespace conrad
{
	namespace synthesis
	{
		/// @brief Support for parallel algorithms using the measurement equation
		///
		/// @details Support for parallel applications using the measurement equation
		/// classes. An application is derived from this abstract base. The model used is that the
		/// application has many prediffers and one solver, running in separate MPI processes
		/// or in one single thread. The solver is the master so the number of processes
		/// is one more than the number of prediffers. Each prediffer is currently given
		/// a separate data set.
		///
		/// The steps are:
		/// (a) define an initial model and distribute to all prediffers
		/// (b) calculate the normal equations for each data set (this part is
		/// distributed across the prediffers)
		/// (c) send all normal equations to the solver for merging
		/// (d) solve the merged normal equations
		/// (e) distribute the model to all prediffers and return to (b)
		///
		/// The caller is responsible for ensuring that the model is transferred correctly 
		/// before a CalcNE and after a SolveNE. For example:
		/// @code
		///		for (int cycle=0;cycle<nCycles;cycle++)
		///		{
		///			imager.os() << "*** Starting major cycle " << cycle << " ***" << std::endl;
		///			if(cycle>0) imager.receiveModel();
		///			imager.calcNE();
		///			imager.solveNE();
		///			// Broadcast the model
		///			if (cycle<(nCycles-1)) imager.broadcastModel();
		///		}
		/// @endcode
		/// The normal equations are transferred automatically between the calcNE and solveNE
		/// steps so the called does not need to be concerned about that.
		///
		/// If the number of nodes is 1 then everything occurs in the same process with
		/// no overall for transmission of model or normal equations.
		///
		/// @ingroup parallel
		class MEParallel : public SynParallel
		{
			public:

				/// @brief Constructor 
				/// @details The command line inputs are needed solely for MPI - currently no
				/// application specific information is passed on the command line.
				/// @param argc Number of command line inputs
				/// @param argv Command line inputs
				MEParallel(int argc, const char** argv);

				~MEParallel();

				/// Calculate the normalequations (runs only in the workers)
				virtual void calcNE() = 0;

				/// Solve the normal equations (runs only in the master)
				virtual void solveNE() = 0;

				/// Write the model (runs only in the master)
				virtual void writeModel() = 0;

				/// @brief Send the normal equations from this worker to the master
				void sendNE();

				/// @brief Receive the normal equations from all workers into this master
				void receiveNE();

			protected:

				/// Holder for the normal equations
				conrad::scimath::NormalEquations::ShPtr itsNe;

				/// Holder for the solver
				conrad::scimath::Solver::ShPtr itsSolver;
				
				/// Holder for the equation
				conrad::scimath::Equation::ShPtr itsEquation;

		};

	}
}
#endif
