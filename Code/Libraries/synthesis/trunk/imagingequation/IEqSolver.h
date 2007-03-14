#ifndef IEQSOLVER_H_
#define IEQSOLVER_H_

#include "IEqParams.h"

namespace conrad
{

class IEqSolver
{
public:
	IEqSolver();
	virtual ~IEqSolver();
	
	// Initialize
	virtual void init()=0;
	
	// Set the params
	virtual bool setParams(IEqParams& ip)=0;
	
	// Return current values of params
	virtual IEqParams& params()=0;
	
	// Add in derivatives
	virtual bool add(IEqParams& ip)=0;
	
	virtual bool solve()=0;
	
};

}

#endif /*IEQSOLVER_H_*/
