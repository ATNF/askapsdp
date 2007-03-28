#include "IEqImageParam.h"

namespace conrad
{
	
IEqImageParam::IEqImageParam(const bool free) : itsFree(free) {};

// Assignment operator
IEqImageParam& IEqImageParam::operator=(const IEqImageParam& other)
{
	if(this!=&other) {
		itsValue=other.itsValue;
		itsDeriv=other.itsDeriv;
		itsDeriv2=other.itsDeriv2;
		itsFree=other.itsFree;
	}
};

// Copy constructor
IEqImageParam::IEqImageParam(const IEqImageParam& other)
{
	operator=(other);
};

