/// @file
///
/// IEqParam: represent a parameter for imaging equation. A parameter
/// can be a single real number, a vector of numbers, or an image of numbers.
/// The first two derivatives may optionally be included.
/// 
/// TODO: use basis functions iso derivatives?
/// TODO: Template instead on double, Image<Float>, Componentlist, etc.?
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef IEQPARAM_H
#define IEQPARAM_H

namespace conrad { 
	
class IEqParam {
public:

	/// Assignment operator
	IEqParam& operator=(const IEqParam& other);
	
	/// Copy constructor
	IEqParam(const IEqParam& other);
	
	/// Single parameter - a parameter has a value and
	/// first and second derivatives. 
	/// @param value value of param
	/// @param deriv first derivative of param
	/// @param deriv2 second derivative of param
	IEqParam(const double value=0.0, 
		const double deriv=0.0, const double deriv2=0.0);
	
	/// Return value of param
	double& value() {return itsValue;};
	const double& value() const {return itsValue;};
	
	/// Return derivative of param
	double& deriv() {return itsDeriv;};
	const double& deriv() const {return itsDeriv;};
	
	/// Return second derivative of param
	double& deriv2() {return itsDeriv2;};
	const double& deriv2() const {return itsDeriv2;};
	
	void fix() {itsFree=true;};
	void free() {itsFree=false;};
	
	const bool fixed() const {return !itsFree;};
	const bool freed() const {return itsFree;};
		
	// Destructor
	virtual ~IEqParam() {};
	
protected:
	double itsValue;
	double itsDeriv;
	double itsDeriv2;
	bool itsFree;
private:

};

};

#endif





