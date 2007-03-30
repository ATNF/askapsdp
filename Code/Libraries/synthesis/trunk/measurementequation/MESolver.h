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
#include "MEImageParams.h"
#include "Iterative.h"

namespace conrad
{

class MESolver : public Iterative
{
public:	

	MESolver(const MEParams& ip, const MEImageParams& iip);

	MESolver() {};
	
	/// Initialize this solver
	virtual void init();
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const MEParams& ip) {
		itsParams=ip;
	}
	void setParameters(const MEImageParams& iip) {
		itsImageParams=iip;
	}
	
	/// Return current values of params
	const MEParams& getParameters() const {return itsParams;};
	const MEImageParams& getImageParameters() const {return itsImageParams;};
	
	MEParams& getParameters() {return itsParams;};
	MEImageParams& getImageParameters() {return itsImageParams;};
	
	/// Add in values for derivatives
	/// @param ip Parameter to set derivatives for
	virtual void addDerivatives(MEParams& ip);
	virtual void addDerivatives(MEImageParams& iip);
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve();
	virtual bool solveImage();
	
protected:
	MEParams itsParams;
	MEImageParams itsImageParams;
};

}

#endif /*MESOLVER_H_*/
