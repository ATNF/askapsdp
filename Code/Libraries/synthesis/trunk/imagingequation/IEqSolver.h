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
	
	// Initialize
	virtual void init() {
		cout << "Stubbed Initialization of solver in " << itsName << endl;
	};
	
	// Set the params
	virtual bool setParameters(IEqParams& ip) {
		itsParams=ip;
	}
	
	// Return current values of params
	virtual IEqParams& parameters() {return itsParams;};
	
	// Add in value and derivatives
	virtual bool add(IEqParams& ip) {
		cout << "Stubbed adding derivatives in " << itsName << endl;
	}
	
	virtual bool solve() {
		cout << "Stubbed solver in " << itsName << endl;
	}

protected:
	casa::String itsName;
	IEqParams itsParams;
};

}

#endif /*IEQSOLVER_H_*/
