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

#include "MESolver.h"
#include <casa/aips.h>
#include <scimath/Fitting/LSQFit.h>

namespace conrad
{

class MESimpleSolver : public MESolver
{
public:	

	MESimpleSolver(const MEParams& ip);
	
	/// Initialize this solver
	virtual void init();
	
	/// Add in normal equations
	/// @param fitter containing normal equations
	virtual void addEquations(const casa::LSQFit& fitter);
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve(MEQuality& q);
	virtual bool solveImage(MEQuality& q);
	
protected:
	casa::LSQFit itsFitter;
};

}

#endif /*MESOLVER_H_*/
