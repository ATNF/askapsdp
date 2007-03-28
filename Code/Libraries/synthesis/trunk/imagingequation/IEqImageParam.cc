#include "IEqImageParam.h"

namespace conrad
{
	
// Assignment operator
IEqImageParam& IEqImageParam::operator=(const IEqImageParam& other)
{
	if(this!=&other) {
		itsValue=other.itsValue;
		itsDeriv=other.itsDeriv;
		itsDeriv2=other.itsDeriv2;
		itsPSF=other.itsPSF;
		itsFree=other.itsFree;
	}
};

// Copy constructor
IEqImageParam::IEqImageParam(const IEqImageParam& other)
{
	operator=(other);
};

}
