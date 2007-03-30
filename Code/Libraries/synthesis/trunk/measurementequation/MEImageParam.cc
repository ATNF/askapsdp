#include "MEImageParam.h"

namespace conrad
{
	
// Assignment operator
MEImageParam& MEImageParam::operator=(const MEImageParam& other)
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
MEImageParam::MEImageParam(const MEImageParam& other)
{
	operator=(other);
};

}
