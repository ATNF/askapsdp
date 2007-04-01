/// @file
///
/// MESolver: Base class for solvers of parametrized imaging
/// equations.
///
/// The base solver does something sensible and can be used as-is.
/// For simple parameters, a least squares solution is calculated,
/// and for image parameters, a steepest descent algorithm is used.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MESOLVER_H_
#define MESOLVER_H_

#include <scimath/Fitting/LSQFit.h>

#include <measurementequation/MEParams.h>
#include <measurementequation/MEIterative.h>
#include <measurementequation/MEQuality.h>

namespace conrad
{

class MESolver : public MEIterative
{
public:	

	MESolver(const MEParams& ip);

	MESolver() {};
	
	/// Initialize this solver
	virtual void init() = 0;
	
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
	
	/// Add in normal equations
	/// @param fitter containing normal equations
	virtual void addEquations(const casa::LSQFit& fitter) = 0;
	
	/// Solve for parameters, updating the values kept internally
	virtual bool solve(MEQuality& q) = 0;
	virtual bool solveImage(MEQuality& q) = 0;
	
protected:
	MEParams itsParams;
	MEImageParams itsImageParams;
};

}

#endif /*MESOLVER_H_*/
