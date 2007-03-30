/// @file
///
/// MEImageParam: represent an image parameter for imaging equation. 
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

#include "MEImage.h"
#include "MEParamBase.h"

namespace conrad {
	
class MEImageParam : public MEParamBase<MEImage> {
public:
	MEImageParam() {};
	MEImageParam(const MEImageParam&);
	MEImageParam& operator=(const MEImageParam&);
	
	// Override definitions for sets to do something sensible for images
	void setValue(const double value) {itsValue.set(value);};
	void setDeriv(const double deriv) {itsDeriv.set(deriv);};
	void setDeriv2(const double deriv2) {itsDeriv2.set(deriv2);};
	
	/// Set and return second derivative of param
	void setPSF(const MEImage& psf) {itsPSF=psf;};
	const MEImage& PSF() const {return itsPSF;};
	void setPSF(const double psf) {itsPSF.set(psf);};
	
protected:
	MEImage itsPSF;
};

};

#endif





