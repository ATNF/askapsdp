#ifndef IEQPARAM_H
#define IEQPARAM_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <images/Images/ImageInterface.h>

namespace conrad { 
	
class IEqParam {
public:

	IEqParam();
	
	// Single parameter
	IEqParam(const casa::String& name, const double value);
	IEqParam(const casa::String& name, const double value, const double deriv);
	
	// Vector of parameters
	IEqParam(const casa::String& name, const casa::Vector<double>& values);
	IEqParam(const casa::String& name, const casa::Vector<double>& values, const casa::Vector<double>& deriv);
	
	// Parameters in an image
	IEqParam(const casa::String& name, const casa::ImageInterface<float>& value);
	IEqParam(const casa::String& name, const casa::ImageInterface<float>& value, const casa::ImageInterface<float>& deriv);
	  
	// Destructor
	virtual ~IEqParam();
	
	// Assignment operator
	IEqParam& operator=(const IEqParam& other);
	
	// Copy constructor
	IEqParam(const IEqParam& other);
	
protected:
};

};

#endif





