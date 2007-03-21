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
		itsValue=other.itsValue;
		itsDeriv=other.itsDeriv;
		itsDeriv2=other.itsDeriv2;
	}
};

IEqParam::IEqParam(const double value, const double deriv, 
	const double deriv2) : itsValue(value), itsDeriv(deriv), itsDeriv2(deriv2)
{};
	
}
