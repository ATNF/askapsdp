/// @file
///
/// quation: Represent a parametrized imaging equation. An quation 
/// is constructed with two sets of parameters. The parameters
/// can be updated subsequently. The quation can do two principal things
///    - calculate data (passed via a data accessor)
///    - transpose residual data back to the parameter space
///
/// These two things can be combined in a calcDerivatives step to allow calculation
/// of gradients for parameters. The parameters may then be solved for by
/// an Solver class.
/// 
/// There are two classes of parameters - regular parameters which are single values
/// doubles andd image parameters (usually a TempImage of 
/// floats with coordinates). The image pixels are treated homogeneously
/// so that a derivative of chi-squared with respect to the image is itself an image.
/// Note that this split is an optimization - logically one could define a regular
/// parameter for each pixel of an image. However, this would be extremely tedious
/// AND inefficient.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNEQUATION_H
#define SYNEQUATION_H

namespace conrad { 

namespace synthesis
{

class IDataAccessor;

class Params;
class NormalEquations;
class NormalEquations;
class DesignMatrix;

class quation {
public:	
	/// Constructor
	/// Using default parameters
    quation() {};
    
    /// Using specified parameters
    quation(const Params& ip) : itsParams(ip) {};
    
    virtual ~quation(){};

	/// Access the parameters
	const Params& parameters() const {return itsParams;};
	Params& parameters() {return itsParams;};
	
	/// Set the parameters to new values
	/// @param ip Parameters
	virtual void setParameters(const Params& ip) {itsParams=ip;};
	
	/// Check if set of parameters is valid for this equation
	/// @param ip Parameters
	virtual bool complete(const Params& ip) {return itsDefaultParams.isCongruent(ip);};
	
	/// Return a default set of parameters
	/// @param ip Parameters
	const Params& defaultParameters() const {return itsDefaultParams;};

	/// Predict model visibility
	/// @param ida data accessor
	virtual void predict(IDataAccessor& ida) = 0;
	
	/// Calculate the design matrix only
	/// @param ida data accessor
	/// @param design matrix
	virtual void calcEquations(IDataAccessor& ida, DesignMatrix& designmatrix) = 0;

	/// Calculate the normal equations
	/// @param ida data accessor
	/// @param normeq Normal equations
	virtual void calcEquations(IDataAccessor& ida, NormalEquations& normeq) = 0;
	
protected:
	Params itsParams;
	Params itsDefaultParams;
};

}
}
#endif





