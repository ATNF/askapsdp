/// @file
///
/// IEqImageSolver: Base class for solvers of parametrized imaging
/// equations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQIMAGESOLVER_H_
#define IEQIMAGESOLVER_H_

#include "IEqImageParams.h"

#include <iostream.h>

namespace conrad
{

class IEqImageSolver
{
public:	
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const IEqImageParams& ip) {
		itsImageParams=ip;
	}
	
	/// Return current values of params
	const IEqImageParams& getParameters() const {return itsImageParams;};
	
	IEqImageParams& parameters() {return itsImageParams;};
	
	/// Add in values for derivatives
	/// @param ip Parameter to set derivatives for
	virtual void addDerivatives(IEqImageParams& ip) = 0;
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve() = 0;
	
protected:
	IEqImageParams itsImageParams;
};

}

#endif /*IEQSOLVER_H_*/
