#include "MEParamsRep.h"

namespace conrad {
	
	template<class T>
	bool MEParamsRep<T>::isFree(const string& name) {
		return itsFree[itsIndices[name]];
	}
	
	template<class T>
	void MEParamsRep<T>::free(const string& name) {
		itsFree[itsIndices[name]]=true;
	}
	
	template<class T>
	void MEParamsRep<T>::fix(const string& name) {
		itsFree[itsIndices[name]]=false;
	}
	
	template<class T>
	void MEParamsRep<T>::add(const string& name, const T& ip) 
	{
		itsValues.push_back(ip);
		uint ind=itsValues.max_size();
		itsIndices.insert(make_pair(name, ind));
		itsFree.push_back(true);
	}
	
	template<class T>
	const vector<T>& MEParamsRep<T>::values() const 
	{
		return itsValues;
	}

	template<class T>
	vector<T>& MEParamsRep<T>::values()
	{
		return itsValues;
	}
	
	template<class T>
	const uint MEParamsRep<T>::nelements() 
	{
		return static_cast<uint>(itsIndices.max_size());
	}
	
	template<class T>
	const T& MEParamsRep<T>::value(const string& name) const 
	{
		return itsValues[itsIndices[name]];
	}		

	template<class T>
	T& MEParamsRep<T>::value(const string& name) 
	{
		return itsValues[itsIndices[name]];
	}

	template<class T>
	const uint MEParamsRep<T>::operator[](const string& name) const 
	{
		return itsIndices[name];
	}	

}