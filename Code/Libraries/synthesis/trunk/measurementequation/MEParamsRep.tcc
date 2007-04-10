#include <measurementequation/MEParamsRep.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>
#include <casa/Exceptions/Error.h>

#include <map>
#include <string>
using std::map;
using std::string;

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
	MEParamsRep<T>& MEParamsRep<T>::operator=(const MEParamsRep& other) {
		if(this!=&other) {
			itsValues=other.itsValues;
			itsFree=other.itsFree;
		}
		return *this;
	}
	
	template<class T>
	bool MEParamsRep<T>::isFree(const string& name) {
		return itsFree[name];
	}
	
	template<class T>
	void MEParamsRep<T>::free(const string& name) {
		itsFree[name]=true;
	}
	
	template<class T>
	void MEParamsRep<T>::fix(const string& name) {
		itsFree[name]=false;
	}
	
	template<class T>
	void MEParamsRep<T>::add(const string& name, const T& ip) 
	{
		if(has(name)) {
			throw(casa::DuplError("Parameter " + name + " already exists"));
		}
		else {
			uint ind=itsValues.size();
			itsValues[name]=ip;
			itsFree[name]=true;
		}
	}
	
	template<class T>
	void MEParamsRep<T>::update(const string& name, const T& ip) 
	{
		if(!has(name)) {
			throw(casa::DuplError("Parameter " + name + " does not already exist"));
		}
		else {
			itsValues[name]=ip;
		}
	}
	
	template<class T>
	const uint MEParamsRep<T>::size() const
	{
		return static_cast<uint>(itsValues.size());
	}
	
	template<class T>
		bool MEParamsRep<T>::has(const string& name) const 
	{
		return itsValues.count(name)>0;
	}		

	template<class T>
		const T& MEParamsRep<T>::value(const string& name) const 
	{
		return itsValues[name];
	}		
	
	template<class T>
		T& MEParamsRep<T>::value(const string& name) 
	{
		return itsValues[name];
	}

	template<class T>
		bool MEParamsRep<T>::isCongruent(const MEParamsRep& other) const
	{
		if(size()!=other.size()) return false;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(other.itsFree.count(iter->first)==0) {
				return false;
			}
		}
		return true;
	}

	template<class T>
		vector<string> MEParamsRep<T>::names() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			names.push_back(iter->first);
		}
		return names;
	}

	template<class T>
		vector<string> MEParamsRep<T>::freeNames() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(itsFree[iter->first]) names.push_back(iter->first);
		}
		return names;
	}

	template<class T>
		vector<string> MEParamsRep<T>::fixedNames() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(!itsFree[iter->first]) names.push_back(iter->first);
		}
		return names;
	}
	
	template<class T>
		vector<string> MEParamsRep<T>::completions(const string& pattern) const
	{
		casa::Regex regex(casa::Regex::fromPattern(pattern));
		vector<string> completions;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(casa::String(iter->first).contains(regex)) completions.push_back(iter->first);
		}
		return completions;
	}

	template<class T>
	void MEParamsRep<T>::reset()
	{
		itsValues.clear();
		itsFree.clear();
	}
}
}