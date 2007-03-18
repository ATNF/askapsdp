/// @file
///
/// IEqParam: represent a parameter for imaging equation. A parameter
/// can be a single real number, a vector of numbers, or an image of numbers.
/// The first two derivatives may optionally be included.
/// 
/// TODO: use basis functions iso derivatives?
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef IEQPARAM_H
#define IEQPARAM_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
//#include <images/Images/ImageInterface.h>

namespace conrad { 
	
class IEqParam {
public:
//	enum Type {DIRECT, IMAGE};

	/// Assignment operator
	IEqParam& operator=(const IEqParam& other);
	
	/// Copy constructor
	IEqParam(const IEqParam& other);
	
	/// Single parameter
	/// @param name Name of param
	/// @param value value of param
	/// @param deriv first derivative of param
	/// @param deriv2 second derivative of param
	IEqParam(const casa::String& name, const double value);
	IEqParam(const casa::String& name, const double value, const double deriv);
	IEqParam(const casa::String& name, const double value, const double deriv, const double deriv2);
	
	/// Return name of param
	casa::String& name() {return itsName;};
	
	/// Return value of param
	double& value() {return itsValue;};
	
	/// Return derivative of param
	double& deriv() {return itsDeriv;};
	
	/// Return second derivative of param
	double& deriv2() {return itsDeriv2;};
	
	void fix() {itsFree=casa::True;};
	void free() {itsFree=casa::False;};
	
	bool fixed() {return !itsFree;};
	bool freed() {return itsFree;};
		
	// Destructor
	virtual ~IEqParam() {};
	
protected:
//	IEqParam::Type itsType;
	casa::String itsName;
	double itsValue;
	double itsDeriv;
	double itsDeriv2;
	bool itsFree;
private:

};

};

#endif





