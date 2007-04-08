#include <measurementequation/MEParamsRep.h>
#include <casa/Exceptions/Error.h>

namespace conrad {
namespace synthesis
{
	
	template<class T>
	MEParamsRep<T>::MEParamsRep() {
	}
	
	template<class T>
	MEParamsRep<T>::MEParamsRep(const MEParamsRep& other) {
		operator=(other);
	}
	
	template<class T>
	MEParamsRep<T> MEParamsRep<T>::operator=(const MEParamsRep& other) {
		if(this!=&other) {
			itsIndices=other.itsIndices;
			itsValues=other.itsValues;
			itsFree=other.itsFree;
		}
	}
	
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
	void MEParamsRep<T>::add(const string& name, const T ip) 
	{
		if(has(name)) {
			throw(casa::DuplError("Parameter " + name + " already exists"));
		}
		else {
			uint ind=itsValues.size();
			itsIndices.insert(make_pair(name, ind));
			itsValues.push_back(ip);
			itsFree.push_back(true);
		}
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
	const uint MEParamsRep<T>::size() const
	{
		return static_cast<uint>(itsIndices.size());
	}
	
	template<class T>
		bool MEParamsRep<T>::has(const string& name) const 
	{
		return itsIndices.count(name)>0;
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
	
	// Test for congruence - to avoid indexing problems, we require that
	// the indices are the same, not just that the same name occurs in
	// both
	template<class T>
	bool MEParamsRep<T>::isCongruent(const MEParamsRep<T>& other) const
	{
		return (itsIndices==other.itsIndices);
	}

	template<class T>
	void MEParamsRep<T>::reset()
	{
		itsValues.clear();
		itsIndices.clear();
		itsFree.clear();
	}
}
}