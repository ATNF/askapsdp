#include "IEqParamsBase.h"

#include <map>
#include <string>
#include <ostream>

using std::string;
using std::map;

namespace conrad
{

template <class T>
const T& IEqParamsBase<T>::operator()(const string& name) const {
	return (*this).find(name)->second;
};
	
template <class T>
T& IEqParamsBase<T>::operator()(const string& name) {
	return (*this).find(name)->second;
};
	
/// Initialize derivatives
template <class T>
void IEqParamsBase<T>::initDerivatives()
{
};

/// Add derivatives
template <class T>
void IEqParamsBase<T>::addDerivatives(const IEqParamsBase& ip)
{
};

template <class T>
void IEqParamsBase<T>::add(const string& name)
{
	T ip;
	this->insert(make_pair(name, ip));
};

template <class T>
void IEqParamsBase<T>::add(const string& name, const T& ip) 
{
	this->insert(make_pair(name, ip));
};
	


}