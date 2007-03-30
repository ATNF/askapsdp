/// @file
///
/// MESolver: Base class for solvers of parametrized imaging
/// equations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MESOLVER_H_
#define MESOLVER_H_

#include "MEParams.h"

#include <iostream.h>

namespace conrad
{

class MESolver
{
public:	
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const MEParams& ip) {
		itsParams=ip;
	}
	
	/// Return current values of params
	const MEParams& getParameters() const {return itsParams;};
	
	MEParams& getParameters() {return itsParams;};
	
	/// Add in values for derivatives
	/// @param ip Parameter to set derivatives for
	virtual void addDerivatives(MEParams& ip) = 0;
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve() = 0;
	
protected:
	MEParams itsParams;
};

}

#endif /*MESOLVER_H_*/
