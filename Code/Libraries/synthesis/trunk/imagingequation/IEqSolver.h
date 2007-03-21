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
	IEqSolver(const string& name, const IEqParams& ip) : itsName(name), itsParams(ip) {};
	
	virtual ~IEqSolver() {
		cout << "Stubbed destruction of solver in " << itsName << endl;
	};
	
	/// Initialize this solver
	virtual void init() {
		cout << "Stubbed initialization of solver in " << itsName << endl;
	};
	
	/// Set the parameters
	/// @param ip Parameters
	virtual void setParameters(const IEqParams& ip) {
		itsParams=ip;
	}
	
	/// Return current values of params
	const virtual IEqParams& parameters() const {return itsParams;};
	
	/// Add in values for derivatives
	/// @param ip Parameter to set derivatives for
	virtual bool add(const IEqParams& ip) {
		cout << "Stubbed adding values and derivatives in " << itsName << endl;
	}
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve() {
		cout << "Stubbed solver in " << itsName << endl;
	}

protected:
	string itsName;
	IEqParams itsParams;
};

}

#endif /*IEQSOLVER_H_*/
