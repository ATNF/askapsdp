/// @file
///
/// MELinearSolver: This solver uses SVD to solve the design matrix
/// equations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MELINEARSOLVER_H_
#define MELINEARSOLVER_H_

#include <measurementequation/MESolver.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEParams.h>

namespace conrad
{
namespace synthesis
{

class MELinearSolver : public MESolver
{
public:	

	MELinearSolver(const MEParams& ip) : MESolver(ip) {};
	
	/// Initialize this solver
	virtual void init();
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the normal equations
	virtual bool solveNormalEquations(MEQuality& q);
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the design matrix
	virtual bool solveDesignMatrix(MEQuality& q);
	
protected:
};

}
}

#endif
