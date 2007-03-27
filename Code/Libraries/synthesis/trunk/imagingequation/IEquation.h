/// @file
///
/// IEquation: Represent a parametrized imaging equation. An IEquation 
/// is constructed with two sets of parameters. The parameters
/// can be updated subsequently. The IEquation can do two principal things
///    - calculate data (passed via a data accessor)
///    - transpose residual data back to the parameter space
///
/// These two things can be combined in a prediffer step to allow calculation
/// of gradients for parameters. The parameters may then be solved for by
/// an IEqSolver class.
/// 
/// There are two classes of parameters - regular parameters which are single values
/// (IEqParam - doubles), and image parameters (IEqImageParam - usually an image of 
///  floats with coordinates). The image pixels are treated homogeneously
/// so that a derivative of chi-squared with respect to the image is itself an image.
/// Note that this split is an optimization - logically one could define a regular
/// parameter for each pixel of an image. However, this would be extremely tedious
/// AND inefficient.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQUATION_H
#define IEQUATION_H

#include "IEqParams.h"
#include "IEqImageParams.h"
#include "IEqDataAccessor.h"

//class IEqDataAccessor;

namespace conrad { 

class IEquation {
public:

	/// Constructor
    IEquation() {};

	/// Predict model visibility
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void predict(const IEqParams& ip, const IEqImageParams& iip, IEqDataAccessor& ida) =0;
	
	/// Transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void transpose(IEqParams& ip, const IEqImageParams& iip, IEqDataAccessor& ida) = 0;
	virtual void transposeImage(const IEqParams& ip, IEqImageParams& iip, IEqDataAccessor& ida) =0;
		
	/// Predict and then transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void prediffer(IEqParams& ip, const IEqImageParams& iip, IEqDataAccessor& ida) =0;
	virtual void predifferImage(const IEqParams& ip, IEqImageParams& iip, IEqDataAccessor& ida) = 0;
	
};

}

#endif





