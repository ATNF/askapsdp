/// @file
///
/// MEImageSolver: Base class for solvers of parametrized imaging
/// equations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEIMAGESOLVER_H_
#define MEIMAGESOLVER_H_

#include "MEImageParams.h"

#include <iostream.h>

namespace conrad
{

class MEImageSolver
{
public:	
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const MEImageParams& ip) {
		itsImageParams=ip;
	}
	
	/// Return current values of params
	const MEImageParams& getParameters() const {return itsImageParams;};
	
	MEImageParams& parameters() {return itsImageParams;};
	
	/// Add in values for derivatives
	/// @param ip Parameter to set derivatives for
	virtual void addDerivatives(MEImageParams& ip) = 0;
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve() = 0;
	
protected:
	MEImageParams itsImageParams;
};

}

#endif /*MESOLVER_H_*/
