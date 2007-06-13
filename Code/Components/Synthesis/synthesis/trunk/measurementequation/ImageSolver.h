/// @file
///
/// ImageSolver: This solver uses SVD to solve the design matrix
/// equations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNIMAGESOLVER_H_
#define SYNIMAGESOLVER_H_

#include <fitting/Solver.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>

namespace conrad
{
namespace synthesis
{

class ImageSolver : public conrad::scimath::Solver
{
public:	

	ImageSolver(const conrad::scimath::Params& ip) : conrad::scimath::Solver(ip) {};
	
	/// Initialize this solver
	virtual void init();
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the normal equations
	/// @param q Solution quality information
	virtual bool solveNormalEquations(conrad::scimath::Quality& q);
		
protected:
};

}
}

#endif
