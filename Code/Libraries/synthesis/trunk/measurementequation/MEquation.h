/// @file
///
/// MEuation: Represent a parametrized imaging equation. An MEuation 
/// is constructed with two sets of parameters. The parameters
/// can be updated subsequently. The MEuation can do two principal things
///    - calculate data (passed via a data accessor)
///    - transpose residual data back to the parameter space
///
/// These two things can be combined in a prediffer step to allow calculation
/// of gradients for parameters. The parameters may then be solved for by
/// an MESolver class.
/// 
/// There are two classes of parameters - regular parameters which are single values
/// (MERealParam - doubles), and image parameters (MEImageParam - usually an image of 
///  floats with coordinates). The image pixels are treated homogeneously
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

#include "MEParams.h"
#include "MEImageParams.h"
#include "MEDataAccessor.h"

namespace conrad { 
	
	class MESolver;
	class MEImageSolver;

class MEquation {
public:

	/// Constructor
    MEquation() {};

	/// Predict model visibility
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void predict(const MEParams& ip, const MEImageParams& iip, MEDataAccessor& ida) =0;
	
	/// Transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida Data accessor
	/// @param is Solver
	/// @param iis Image solver
	virtual void transpose(MEParams& ip, const MEImageParams& iip, MEDataAccessor& ida,
		MESolver& is) = 0;
	virtual void transpose(const MEParams& ip, MEImageParams& iip, MEDataAccessor& ida, 
		MEImageSolver& iis) = 0;
	virtual void transpose(MEParams& ip, MEImageParams& iip, MEDataAccessor& ida, 
		MESolver& is, MEImageSolver& iis) = 0;
				
	/// Predict and then transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void prediffer(MEParams& ip, const MEImageParams& iip, MEDataAccessor& ida,
		MESolver& is) = 0;
	virtual void prediffer(const MEParams& ip, MEImageParams& iip, MEDataAccessor& ida,
		MEImageSolver& iis) = 0;
	virtual void prediffer(MEParams& ip, MEImageParams& iip, MEDataAccessor& ida,
		MESolver& is, MEImageSolver& iis) = 0;
	
};

}

#endif





