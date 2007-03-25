/// @file
///
/// IEqSolver: Base class for solvers of parametrized imaging
/// equations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQSOLVER_H_
#define IEQSOLVER_H_

#include "IEqParams.h"

#include <iostream.h>

namespace conrad
{

class IEqSolver
{
public:	
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const IEqParams& ip) {
		itsParams=ip;
	}
	
	/// Return current values of params
	const IEqParams& parameters() const {return itsParams;};
	
	IEqParams& parameters() {return itsParams;};
	
	/// Add in values for derivatives
	/// @param ip Parameter to set derivatives for
	virtual void addDerivatives(IEqParams& ip) = 0;
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve() = 0;
	
protected:
	IEqParams itsParams;
};

}

#endif /*IEQSOLVER_H_*/
