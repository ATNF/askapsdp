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

#ifndef MEIMAGEPARAM_H
#define MEIMAGEPARAM_H

#include <lattices/Lattices/LatticeExprNode.h>

#include "MEImage.h"
#include "MEParamBase.h"

namespace conrad {
	
class MEImageParam : public MEParamBase<MEImage> {
public:
	MEImageParam() {};
	MEImageParam(const MEImageParam&);
	MEImageParam& operator=(const MEImageParam&);
	
	// Override definitions for sets to do something sensible for images
	virtual void setValue(const MEImage& value) {itsValue.copyData(value);};
	virtual void setDeriv(const MEImage& deriv) {itsDeriv.copyData(deriv);};
	virtual void setDeriv2(const MEImage& deriv2) {itsDeriv2.copyData(deriv2);};
	virtual void setPSF(const MEImage& psf) {itsPSF.copyData(psf);};
	
	virtual void setValue(const double value) {itsValue.set(value);};
	virtual void setDeriv(const double deriv) {itsDeriv.set(deriv);};
	virtual void setDeriv2(const double deriv2) {itsDeriv2.set(deriv2);};
	virtual void setPSF(const double psf) {itsPSF.set(psf);};
	
	/// Set and return second derivative of param
	virtual const MEImage& PSF() const {return itsPSF;};
	
protected:
	MEImage itsPSF;
};

};

#endif





