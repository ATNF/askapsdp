/// @file
///
/// LinearSolver: This solver uses SVD to solve the design matrix
/// equations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNLINEARSOLVER_H_
#define SYNLINEARSOLVER_H_

#include <fitting/Solver.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>

namespace conrad
{
namespace synthesis
{

class LinearSolver : public Solver
{
public:	

	LinearSolver(const Params& ip) : Solver(ip) {};
	
	/// Initialize this solver
	virtual void init();
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the normal equations
	/// @param q Quality information
	/// @param useSVD use SVD instead of Cholesky decomposition
	virtual bool solveNormalEquations(Quality& q, const bool useSVD=false);
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the design matrix
	virtual bool solveDesignMatrix(Quality& q);
	
protected:
};

}
}

#endif
