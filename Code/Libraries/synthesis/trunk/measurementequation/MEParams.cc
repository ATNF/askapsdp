#include <measurementequation/MEParams.h>
#include <measurementequation/MEDomain.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>

#include <map>
#include <string>
#include <stdexcept>
using std::map;
using std::string;

namespace conrad {
namespace synthesis
{
	
	MEParams::MEParams() {
	}
	
	MEParams::MEParams(const MEParams& other) {
		operator=(other);
	}
	
	MEParams& MEParams::operator=(const MEParams& other) {
		if(this!=&other) {
			itsArrays=other.itsArrays;
			itsDomains=other.itsDomains;
			itsFree=other.itsFree;
		}
		return *this;
	}
	
	bool MEParams::isFree(const string& name) const {
		return itsFree[name];
	}
	
	void MEParams::free(const string& name) {
		itsFree[name]=true;
	}
	
	void MEParams::fix(const string& name) {
		itsFree[name]=false;
	}	

	void MEParams::add(const string& name, const double ip) 
	{
		if(has(name)) {
			throw(std::invalid_argument("Parameter " + name + " already exists"));
		}
		else {
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsDomains[name]=MEDomain();
		}
	}
	
	void MEParams::add(const string& name, const casa::Array<double>& ip) 
	{
		if(has(name)) {
			throw(std::invalid_argument("Parameter " + name + " already exists"));
		}
		else {
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsDomains[name]=MEDomain();
		}
	}
	
	void MEParams::add(const string& name, const casa::Array<double>& ip,
		const MEDomain& domain)
	{
		if(has(name)) {
			throw(std::invalid_argument("Parameter " + name + " already exists"));
		}
		else {
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsDomains[name]=domain;
		}
	}
	
	void MEParams::add(const string& name, const double ip, const MEDomain& domain) 
	{
		if(has(name)) {
			throw(std::invalid_argument("Parameter " + name + " already exists"));
		}
		else {
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsDomains[name]=domain;
		}
	}
	
	void MEParams::update(const string& name, const casa::Array<double>& ip) 
	{
		if(!has(name)) {
			throw(std::invalid_argument("Parameter " + name + " does not already exist"));
		}
		else {
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsDomains[name]=MEDomain();
		}
	}
	
	void MEParams::update(const string& name, const double ip) 
	{
		if(!has(name)) {
			throw(std::invalid_argument("Parameter " + name + " does not already exist"));
		}
		else {
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsDomains[name]=MEDomain();
		}
	}
	
	const uint MEParams::size() const
	{
		return static_cast<uint>(itsFree.size());
	}
	
	bool MEParams::has(const string& name) const 
	{
		return itsArrays.count(name)>0;
	}		

	bool MEParams::isScalar(const string& name) const 
	{
		return itsArrays[name].shape().isEqual(casa::IPosition(1,1));
	}		

	const casa::Array<double>& MEParams::value(const string& name) const 
	{
		return itsArrays[name];
	}		
	
	casa::Array<double>& MEParams::value(const string& name) 
	{
		return itsArrays[name];
	}		
	
	const MEDomain& MEParams::domain(const string& name) const 
	{
		return itsDomains[name];
	}		
	
	MEDomain& MEParams::domain(const string& name) 
	{
		return itsDomains[name];
	}		
	
	bool MEParams::isCongruent(const MEParams& other) const
	{
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(other.itsFree.count(iter->first)==0) {
				return false;
			}
		}
		return true;
	}

	vector<string> MEParams::names() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			names.push_back(iter->first);
		}
		return names;
	}

	vector<string> MEParams::freeNames() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(itsFree[iter->first]) names.push_back(iter->first);
		}
		return names;
	}

	vector<string> MEParams::fixedNames() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(!itsFree[iter->first]) names.push_back(iter->first);
		}
		return names;
	}
	
	vector<string> MEParams::completions(const string& pattern) const
	{
		casa::Regex regex(casa::Regex::fromPattern(pattern));
		vector<string> completions;
		std::map<string,bool>::iterator iter;
		uint ncomplete=0;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(casa::String(iter->first).matches(regex)) {
				completions.push_back(iter->first);
				ncomplete++;
			}
		}
		return completions;
	}

	void MEParams::reset()
	{
		itsArrays.clear();
		itsDomains.clear();
		itsFree.clear();
	}
}
}
