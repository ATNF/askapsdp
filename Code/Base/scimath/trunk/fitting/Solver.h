/// @file
///
/// Solver: Base class for solvers of parametrized imaging
/// equations.
///
/// The base solver does something sensible and can be used as-is.
/// For simple parameters, a least squares solution is calculated,
/// and for image parameters, a steepest descent algorithm is used.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHSOLVER_H_
#define SCIMATHSOLVER_H_

#include <fitting/Params.h>
#include <fitting/NormalEquations.h>
#include <fitting/Solveable.h>
#include <fitting/Quality.h>

namespace conrad
{
namespace scimath
{

class Solver : public Solveable
{
public:	
    /// Constructor from parameters
	explicit Solver(const Params& ip);

	virtual ~Solver() {};
	
	/// Initialize this solver
	virtual void init() = 0;
	
	/// Set the parameters
	/// @param ip Parameters
	void setParameters(const Params& ip);

	/// Return current values of params
	const Params& parameters() const;
	Params& parameters();
			
	/// Add the normal equations
	/// @param normeq Normal Equations
	virtual void addNormalEquations(const NormalEquations& normeq);

	/// Solve for parameters, updating the values kept internally
	/// The solution is constructed from the normal equations
	virtual bool solveNormalEquations(Quality& q) = 0;
	
protected:
	Params itsParams;
	NormalEquations itsNormalEquations;
};

}
}

#endif /*SOLVER_H_*/
