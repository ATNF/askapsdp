/// @file
///
/// LinearSolver: This solver uses SVD to solve the normal
/// equations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHLINEARSOLVER_H_
#define SCIMATHLINEARSOLVER_H_

#include <fitting/Solver.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>

namespace conrad
{
namespace scimath
{

class LinearSolver : public Solver
{
public:	
    /// Constructor
	explicit LinearSolver(const Params& ip) : Solver(ip) {};
	
	/// Initialize this solver
	virtual void init();
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the normal equations
	/// @param q Quality information
	virtual bool solveNormalEquations(Quality& q);
	
protected:
};

}
}

#endif
