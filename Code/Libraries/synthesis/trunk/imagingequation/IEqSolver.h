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

#include <casa/aips.h>
#include <casa/BasicSL/String.h>

#include "IEqParams.h"

#include <iostream.h>

namespace conrad
{

class IEqSolver
{
public:
	IEqSolver(casa::String name, IEqParams& ip) : itsName(name), itsParams(ip) {};
	
	virtual ~IEqSolver() {};
	
	/// Initialize this solver
	virtual void init() {
		cout << "Stubbed initialization of solver in " << itsName << endl;
	};
	
	/// Set the parameters
	/// @param ip Parameters
	virtual bool setParameters(IEqParams& ip) {
		itsParams=ip;
	}
	
	/// Return current values of params
	virtual IEqParams& parameters() {return itsParams;};
	
	/// Add in a new parameter
	/// @param ip New parameter to set
	virtual bool add(IEqParams& ip) {
		cout << "Stubbed adding derivatives in " << itsName << endl;
	}
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve() {
		cout << "Stubbed solver in " << itsName << endl;
	}

protected:
	casa::String itsName;
	IEqParams itsParams;
};

}

#endif /*IEQSOLVER_H_*/
