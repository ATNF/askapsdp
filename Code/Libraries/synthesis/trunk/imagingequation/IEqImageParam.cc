#include "IEqImageParam.h"

namespace conrad
{

// Assignment operator
IEqImageParam& IEqImageParam::operator=(const IEqImageParam& other)
{
	operator=(other);
};

// Copy constructor
IEqImageParam::IEqImageParam(const IEqImageParam& other)
{
	if(this!=&other) {
		itsValue=other.itsValue;
		itsDeriv=other.itsDeriv;
		itsDeriv2=other.itsDeriv2;
		itsFree=other.itsFree;
	}
};

IEqImageParam::IEqImageParam(const bool free) : itsFree(free)
{
	itsValue.set(0.0);
	itsDeriv.set(0.0);
	itsDeriv2.set(0.0);
};

IEqImageParam::IEqImageParam(const IEqImage& value, const bool free) : itsValue(value), itsFree(free)
{
	itsDeriv.set(0.0);
	itsDeriv2.set(0.0);
};

IEqImageParam::IEqImageParam(const IEqImage& value, const IEqImage& deriv, 
	const bool free) : itsValue(value), itsDeriv(deriv), itsFree(free)
{
	itsDeriv2.set(0.0);
};


IEqImageParam::IEqImageParam(const IEqImage& value, const IEqImage& deriv, 
	const IEqImage& deriv2, const bool free) : itsValue(value), itsDeriv(deriv), itsDeriv2(deriv2), itsFree(free)
{};
	
}
