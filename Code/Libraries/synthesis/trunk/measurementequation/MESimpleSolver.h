/// @file
///
/// MESimpleSolver: This simple solver does something sensible and simple.
/// For real parameters, a least squares solution is calculated,
/// and for image parameters, a steepest descent algorithm is used.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MESIMPLESOLVER_H_
#define MESIMPLESOLVER_H_

#include <measurementequation/MERegularSolver.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEParams.h>

namespace conrad
{
namespace synthesis
{

class MESimpleSolver : public MERegularSolver
{
public:	

	MESimpleSolver(const MERegularParams& ip) : MERegularSolver(ip) {};
	
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

#endif /*MESOLVER_H_*/
