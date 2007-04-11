/// @file
///
/// MERegularSolver: Base class for solvers of parametrized imaging
/// equations.
///
/// The base solver does something sensible and can be used as-is.
/// For simple parameters, a least squares solution is calculated,
/// and for image parameters, a steepest descent algorithm is used.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEREGULARSOLVER_H_
#define MEREGULARSOLVER_H_

#include <measurementequation/MEParams.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEIterative.h>
#include <measurementequation/MEQuality.h>

namespace conrad
{
namespace synthesis
{

class MERegularSolver : public MEIterative
{
public:	

	MERegularSolver(const MERegularParams& ip);

	virtual ~MERegularSolver() {};
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const MERegularParams& ip);

	/// Return current values of params
	const MERegularParams& getParameters() const;
	
	/// Add in normal equations
	/// @param normeq Normal Equations
	virtual void addNormalEquations(const MERegularNormalEquations& normeq);
	
	/// Add the design matrix
	/// @param designmatrix Design Matrix
	virtual void addDesignMatrix(const MERegularDesignMatrix& designmatrix);
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the normal equations
	virtual bool solveNormalEquations(MEQuality& q) = 0;
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the design matrix
	virtual bool solveDesignMatrix(MEQuality& q) = 0;
	
protected:
	MERegularParams itsParams;
	MERegularNormalEquations itsNormalEquations;
	MERegularDesignMatrix itsDesignMatrix;
};

}
}

#endif /*MESOLVER_H_*/
