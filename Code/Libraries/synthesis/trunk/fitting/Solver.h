/// @file
///
/// Solver: Base class for solvers of parametrized imaging
/// equations.
///
/// The base solver does something sensible and can be used as-is.
/// For simple parameters, a least squares solution is calculated,
/// and for image parameters, a steepest descent algorithm is used.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNSOLVER_H_
#define SYNSOLVER_H_

#include <fitting/Params.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Iterative.h>
#include <fitting/Quality.h>

namespace conrad
{
namespace synthesis
{

class Solver : public Iterative
{
public:	

	Solver(const Params& ip);

	virtual ~Solver() {};
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const Params& ip);

	/// Return current values of params
	const Params& parameters() const;
	Params& parameters();
		
	/// Add the design matrix
	/// @param designmatrix Design Matrix
	virtual void addDesignMatrix(const DesignMatrix& designmatrix);
	
	/// Add the normal equations
	/// @param normeq Normal Equations
	virtual void addNormalEquations(const NormalEquations& normeq);

	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the normal equations
	virtual bool solveNormalEquations(Quality& q) = 0;
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the design matrix
	virtual bool solveDesignMatrix(Quality& q) = 0;
	
protected:
	Params itsParams;
	NormalEquations itsNormalEquations;
	DesignMatrix itsDesignMatrix;
};

}
}

#endif /*SOLVER_H_*/
