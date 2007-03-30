#include "MEParamBase.h"

namespace conrad
{
	
// Assignment operator
template<class T>
MEParamBase<T>& MEParamBase<T>::operator=(const MEParamBase<T>& other)
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
MEParamBase<T>::MEParamBase(const MEParamBase<T>& other)
{
	operator=(other);
};

template<class T>
MEParamBase<T>::MEParamBase(const T& value, const T& deriv, 
	const T& deriv2, const bool free) : itsValue(value), itsDeriv(deriv), itsDeriv2(deriv2), 
	itsFree(free)
{};
	
}
