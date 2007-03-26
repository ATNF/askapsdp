/// @file
///
/// IEqImageParam: represent a parameter for imaging equation. A parameter
/// can be a single real number, a vector of numbers, or an image of numbers.
/// The first two derivatives may optionally be included.
/// 
/// TODO: use basis functions iso derivatives?
/// TODO: Template instead on IEqImage, Image<Float>, Componentlist, etc.?
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef IEQIMAGEPARAM_H
#define IEQIMAGEPARAM_H

#include <ostream>

#include <images/Images/TempImage.h>

typedef casa::TempImage<casa::Float> IEqImage;

namespace conrad { 
	
class IEqImageParam {
public:

	/// Default constructor
	IEqImageParam (const bool=true);
	
	/// Assignment operator
	IEqImageParam& operator=(const IEqImageParam& other);
	
	/// Copy constructor
	IEqImageParam(const IEqImageParam& other);
	
	/// Single parameter - a parameter has a value and
	/// first and second derivatives. 
	/// @param value value of param
	/// @param deriv first derivative of param
	/// @param deriv2 second derivative of param
	IEqImageParam(const IEqImage& value, const bool free=true);
	IEqImageParam(const IEqImage& value, 
		const IEqImage& deriv, const bool free=true);
	IEqImageParam(const IEqImage& value, 
		const IEqImage& deriv, const IEqImage& deriv2, const bool free=true);
	
	/// Return value of param
	void setValue(const IEqImage& value) {itsValue=value;};
	const IEqImage& value() const {return itsValue;};
	
	/// Return derivative of param
	void setDeriv(const IEqImage& deriv) {itsDeriv=deriv;};
	void setDeriv(const float& deriv=0.0) {itsDeriv.set(deriv);};
	const IEqImage& deriv() const {return itsDeriv;};
	
	/// Return second derivative of param
	void setDeriv2(const IEqImage& deriv2) {itsDeriv2=deriv2;};
	void setDeriv2(const float& deriv2=0.0) {itsDeriv2.set(deriv2);};
	const IEqImage& deriv2() const {return itsDeriv2;};
	
	void fix() {itsFree=false;};
	void free() {itsFree=true;};
	
	const bool isFixed() const {return !itsFree;};
	const bool isFree() const {return itsFree;};
		
	// Destructor
	virtual ~IEqImageParam() {};

	// Write out an IEqImageParam
	friend std::ostream& operator<<(std::ostream& os, const IEqImageParam& ip) {
//	  os << "value: " << ip.value() << " derivatives: " << ip.deriv() << " " << ip.deriv2();
	  if (ip.isFree()) {
	    os << " (Free)";
	  }
	  else {
	    os << " (Fixed)";
	  }
	  return os;
	}
	
protected:
	IEqImage itsValue;
	IEqImage itsDeriv;
	IEqImage itsDeriv2;
	bool itsFree;
private:

};


};

#endif





