/// @file
///
/// MESolver: Base class for solvers of parametrized imaging
/// equations.
///
/// The base solver does something sensible and can be used as-is.
/// For simple parameters, a least squares solution is calculated,
/// and for image parameters, a steepest descent algorithm is used.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MESOLVER_H_
#define MESOLVER_H_

#include <measurementequation/MEParams.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEIterative.h>
#include <measurementequation/MEQuality.h>

namespace conrad
{
namespace synthesis
{

class MESolver : public MEIterative
{
public:	

	MESolver(const MEParams& ip);

	virtual ~MESolver() {};
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const MEParams& ip);

	/// Return current values of params
	const MEParams& parameters() const;
	MEParams& parameters();
		
	/// Add the design matrix
	/// @param designmatrix Design Matrix
	virtual void addDesignMatrix(const MEDesignMatrix& designmatrix);
	
	/// Add the normal equations
	/// @param normeq Normal Equations
	virtual void addNormalEquations(const MENormalEquations& normeq);

	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the normal equations
	virtual bool solveNormalEquations(MEQuality& q) = 0;
	
	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the design matrix
	virtual bool solveDesignMatrix(MEQuality& q) = 0;
	
protected:
	MEParams itsParams;
	MENormalEquations itsNormalEquations;
	MEDesignMatrix itsDesignMatrix;
};

}
}

#endif /*MESOLVER_H_*/
