/// @file
///
/// MEImageSolver: Base class for solvers of parametrized imaging
/// equations.
///
/// The base solver does something sensible and can be used as-is.
/// For simple parameters, a least squares solution is calculated,
/// and for image parameters, a steepest descent algorithm is used.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEIMAGESOLVER_H_
#define MEIMAGESOLVER_H_

#include <measurementequation/MEParams.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEIterative.h>
#include <measurementequation/MEQuality.h>

namespace conrad
{
namespace synthesis
{

class MEImageSolver : public MEIterative
{
public:	

	MEImageSolver(const MEImageParams& ip);

	virtual ~MEImageSolver() {};
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const MEImageParams& ip);

	/// Return current values of params
	const MEImageParams& getParameters() const;
	
	/// Add in normal equations
	/// @param normeq Normal Equations
	virtual void addNormalEquations(const MEImageNormalEquations& normeq);
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solveNormalEquations(MEQuality& q) = 0;
	
protected:
	MEImageParams itsParams;
	MEImageNormalEquations itsNormalEquations;
};

}
}

#endif
