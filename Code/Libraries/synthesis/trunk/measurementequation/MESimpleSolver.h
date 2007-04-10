/// @file
///
/// MESimpleSolver: This simple solver does something sensible and simple.
/// For real parameters, a least squares solution is calculated,
/// and for image parameters, a steepest descent algorithm is used.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MESIMPLESOLVER_H_
#define MESIMPLESOLVER_H_

#include <measurementequation/MESolver.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEParams.h>

namespace conrad
{
namespace synthesis
{

class MESimpleSolver : public MESolver
{
public:	

	MESimpleSolver(const MEParams& ip) : MESolver(ip), 
		itsRegularEquations(ip.regular()),  itsImageEquations(ip.image()), 
		itsMatrix(ip.regular()) {};
	
	/// Initialize this solver
	virtual void init();
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const MEParams& ip);

	/// Return current values of params
	const MEParams& getParameters() const;
	
	/// Add in normal equations
	/// @param normeq Normal Equations
	virtual void addEquations(const MEImageNormalEquations& normeq);
	
	/// Add in normal equations
	/// @param normeq Normal Equations
	virtual void addEquations(const MERegularNormalEquations& normeq);
	
	/// Set the design matrix
	/// @param designmatrix Design Matrix
	virtual void setDesignMatrix(const MERegularDesignMatrix& designmatrix);
	
	/// Solve for parameters
	virtual bool solve(MEQuality& q);
	virtual bool solveImage(MEQuality& q);
	
protected:
	MERegularNormalEquations itsRegularEquations;
	MEImageNormalEquations itsImageEquations;
	MERegularDesignMatrix itsMatrix;
};

}
}

#endif /*MESOLVER_H_*/
