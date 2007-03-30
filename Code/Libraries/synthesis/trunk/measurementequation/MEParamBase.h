/// @file
///
/// MEParamBase: template class to represent a parameter for imaging equation. 
/// A parameter can be a single real number or an image. The first two derivatives 
/// are also included.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef MEPARAMBASE_H
#define MEPARAMBASE_H

#include <ostream>

namespace conrad { 
	
template<class T> 
class MEParamBase {
public:

	MEParamBase() : itsFree(true) {};
	MEParamBase(const bool free) : itsFree(free) {};

	/// Assignment operator
	virtual MEParamBase& operator=(const MEParamBase& other);
	
	/// Copy constructor
	MEParamBase(const MEParamBase& other);
	
	/// Single parameter - a parameter has a value and
	/// first and second derivatives. 
	/// @param value value of param
	/// @param deriv first derivative of param
	/// @param deriv2 second derivative of param
	//MEParamBase(const bool free=true);
	MEParamBase(const T& value, const bool free=true);
	MEParamBase(const T& value, const T& deriv, const bool free=true);
	MEParamBase(const T& value, const T& deriv, const T& deriv2, const bool free=true);
	
	/// Set and return value of param
	virtual void setValue(const T& value) {itsValue=value;};
	virtual const T& value() const {return itsValue;};
	
	/// Set and return derivative of param
	virtual void setDeriv(const T& deriv) {itsDeriv=deriv;};
	virtual const T& deriv() const {return itsDeriv;};
	
	/// Set and return second derivative of param
	virtual void setDeriv2(const T& deriv2) {itsDeriv2=deriv2;};
	virtual const T& deriv2() const {return itsDeriv2;};
	
	void fix() {itsFree=false;};
	void free() {itsFree=true;};
	
	const bool isFixed() const {return !itsFree;};
	const bool isFree() const {return itsFree;};
		
	// Destructor
	virtual ~MEParamBase() {};
	
protected:
	T itsValue;
	T itsDeriv;
	T itsDeriv2;
	bool itsFree;
private:

};


};

#endif





