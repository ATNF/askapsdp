/// @file
///
/// IEqImageParam: represent an image parameter for imaging equation. 
/// An image can have derivatives (first and second). Only the 
/// diagonal elements of the second derivative are present. An estimate
/// of the off-diagonal elements is present in the PSF.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef IEQIMAGEPARAM_H
#define IEQIMAGEPARAM_H

#include "IEqImage.h"
#include "IEqParamBase.h"

namespace conrad {
	
class IEqImageParam : public IEqParamBase<IEqImage> {
public:
	IEqImageParam() {};

	void setValue(const double value) {itsValue.set(value);};
	void setDeriv(const double deriv) {itsDeriv.set(deriv);};
	void setDeriv2(const double deriv2) {itsDeriv2.set(deriv2);};
	
};

};

#endif





