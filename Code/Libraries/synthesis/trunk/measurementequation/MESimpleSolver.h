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

#include <measurementequation/MESolver.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>

namespace conrad
{

class MESimpleSolver : public MESolver
{
public:	

	MESimpleSolver(const MEParams& ip) : MESolver(ip) {};
	
	/// Initialize this solver
	virtual void init();
	
	/// Add in normal equations
	/// @param normeq Normal Equations
	virtual void addEquations(const MENormalEquations& normeq);
	
	/// Set the design matrix
	/// @param designmatrix Design Matrix
	virtual void setDesignMatrix(const MEDesignMatrix& designmatrix);
	
	/// Solve for parameters
	virtual bool solve(MEQuality& q);
	virtual bool solveImage(MEQuality& q);
	
protected:
	MENormalEquations itsEquations;
	MEDesignMatrix itsMatrix;
};

}

#endif /*MESOLVER_H_*/
