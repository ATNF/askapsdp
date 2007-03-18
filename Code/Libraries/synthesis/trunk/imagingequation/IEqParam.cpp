#include "IEqParam.h"

namespace conrad
{

// Assignment operator
IEqParam& IEqParam::operator=(const IEqParam& other)
{
	operator=(other);
};

// Copy constructor
IEqParam::IEqParam(const IEqParam& other)
{
	if(this!=&other) {
		itsName=other.itsName;
		itsValue=other.itsValue;
		itsDeriv=other.itsDeriv;
		itsDeriv2=other.itsDeriv2;
	}
};

IEqParam::IEqParam(const casa::String& name, const double value) : itsName(name), itsValue(value),
itsDeriv(0.0), itsDeriv2(0.0)
{};

IEqParam::IEqParam(const casa::String& name, const double value, const double deriv) : itsName(name), 
itsValue(value), itsDeriv(deriv), itsDeriv2(0.0)
{};

IEqParam::IEqParam(const casa::String& name, const double value, const double deriv, 
	const double deriv2) : itsName(name), itsValue(value), itsDeriv(deriv), itsDeriv2(deriv2)
{};
	
}
