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

class MEParams;
class MEDataAccessor;
class MENormalEquations;
class MEDesignMatrix;

class MEquation {
public:	
	/// Constructor
    MEquation() {};

	/// Predict model visibility
	/// @param ip Regular parameters
	/// @param ida data accessor
	virtual void predict(const MEParams& ip, MEDataAccessor& ida) = 0;
	
	/// Calculate the normal equations
	/// @param ip Regular parameters
	/// @param ida data accessor
	/// @param normeq Normal equations
	virtual void calcNormalEquations(MEParams& ip, MEDataAccessor& ida,
		MENormalEquations& normeq) = 0;
	
	/// Calculate the design matrix
	/// @param ip Regular parameters
	/// @param ida data accessor
	/// @param design matrix
	virtual void calcDesignMatrix(MEParams& ip, MEDataAccessor& ida,
		MEDesignMatrix& designmatrix) = 0;
	
};

}

#endif





