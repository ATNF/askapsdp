#include "IEqParamBase.h"

namespace conrad
{

// Assignment operator
template<class T>
IEqParamBase<T>& IEqParamBase<T>::operator=(const IEqParamBase<T>& other)
{
	if(this!=&other) {
		itsValue=other.itsValue;
		itsDeriv=other.itsDeriv;
		itsDeriv2=other.itsDeriv2;
		itsFree=other.itsFree;
	}
};

// Copy constructor
template<class T>
IEqParamBase<T>::IEqParamBase(const IEqParamBase<T>& other)
{
	operator=(other);
};

template<class T>
IEqParamBase<T>::IEqParamBase(const T& value, const T& deriv, 
	const T& deriv2, const bool free) : itsValue(value), itsDeriv(deriv), itsDeriv2(deriv2), 
	itsFree(free)
{};
	
}
