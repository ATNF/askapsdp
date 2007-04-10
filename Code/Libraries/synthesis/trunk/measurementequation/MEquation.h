/// @file
///
/// MEquation: Represent a parametrized imaging equation. An MEquation 
/// is constructed with two sets of parameters. The parameters
/// can be updated subsequently. The MEquation can do two principal things
///    - calculate data (passed via a data accessor)
///    - transpose residual data back to the parameter space
///
/// These two things can be combined in a calcDerivatives step to allow calculation
/// of gradients for parameters. The parameters may then be solved for by
/// an MESolver class.
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
#ifndef MEQUATION_H
#define MEQUATION_H

namespace conrad { 

namespace synthesis
{

class IDataAccessor;

class MEParams;
class MERegularNormalEquations;
class MEImageNormalEquations;
class MERegularDesignMatrix;

class MEquation {
public:	
	/// Constructor
	/// Using default parameters
    MEquation() {};
    
    /// Using specified parameters
    MEquation(const MEParams& ip) : itsParams(ip) {};
    
    virtual ~MEquation(){};

	/// Access the parameters
	const MEParams& parameters() const {return itsParams;};
	MEParams& parameters() {return itsParams;};
	
	/// Set the parameters to new values
	/// @param ip Parameters
	virtual void setParameters(const MEParams& ip) {itsParams=ip;};
	
	/// Check if set of parameters is valid for this equation
	/// @param ip Parameters
	virtual bool complete(const MEParams& ip) {return itsDefaultParams.isCongruent(ip);};
	
	/// Return a default set of parameters
	/// @param ip Parameters
	const MEParams& defaultParameters() const {return itsDefaultParams;};

	/// Predict model visibility
	/// @param ida data accessor
	virtual void predict(IDataAccessor& ida) = 0;
	
	/// Calculate the image normal equations
	/// @param ida data accessor
	/// @param normeq Normal equations
	virtual void calcEquations(IDataAccessor& ida, MEImageNormalEquations& normeq) = 0;
	
	/// Calculate the regular normal equations
	/// @param ida data accessor
	/// @param normeq Normal equations
	virtual void calcEquations(IDataAccessor& ida, MERegularNormalEquations& normeq) = 0;
	
	/// Calculate the regular design matrix and normal equations
	/// @param ida data accessor
	/// @param design matrix
	/// @param normeq Normal equations
	virtual void calcEquations(IDataAccessor& ida,
		MERegularDesignMatrix& designmatrix, MERegularNormalEquations& normeq) = 0;

	/// Calculate the regular design matrix
	/// @param ida data accessor
	/// @param design matrix
	virtual void calcEquations(IDataAccessor& ida,
		MERegularDesignMatrix& designmatrix) = 0;

protected:
	MEParams itsParams;
	MEParams itsDefaultParams;
};

}
}
#endif





