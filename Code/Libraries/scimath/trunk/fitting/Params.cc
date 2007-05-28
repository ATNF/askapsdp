#include <fitting/Params.h>
#include <fitting/Domain.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>

#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
using std::map;
using std::string;
using std::ostream;

namespace conrad {
namespace scimath
{
	
	Params::Params() {
	}
	
	Params::Params(const Params& other) {
		operator=(other);
	}
	
	Params& Params::operator=(const Params& other) {
		if(this!=&other) {
			itsArrays=other.itsArrays;
			itsDomains=other.itsDomains;
			itsFree=other.itsFree;
            itsCounts=other.itsCounts;
		}
		return *this;
	}
	
	bool Params::isFree(const string& name) const {
		return itsFree[name];
	}
	
	void Params::free(const string& name) {
		itsFree[name]=true;
	}
	
	void Params::fix(const string& name) {
		itsFree[name]=false;
	}	

	void Params::add(const string& name, const double ip) 
	{
		if(has(name)) {
			throw(std::invalid_argument("Parameter " + name + " already exists"));
		}
		else {
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsDomains[name]=Domain();
            itsCounts[name]=0;
		}
	}
	
	void Params::add(const string& name, const casa::Array<double>& ip) 
	{
		if(has(name)) {
			throw(std::invalid_argument("Parameter " + name + " already exists"));
		}
        else {
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsDomains[name]=Domain();
            itsCounts[name]=0;
		}
	}
	
	void Params::add(const string& name, const casa::Array<double>& ip,
		const Domain& domain)
	{
		if(has(name)) {
			throw(std::invalid_argument("Parameter " + name + " already exists"));
		}
		else {
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsDomains[name]=domain;
            itsCounts[name]=0;
		}
	}
	
	void Params::add(const string& name, const double ip, const Domain& domain) 
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
            itsCounts[name]=0;
		}
	}
	
	void Params::update(const string& name, const casa::Array<double>& ip) 
	{
		if(!has(name)) {
			throw(std::invalid_argument("Parameter " + name + " does not already exist"));
		}
		else {
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsDomains[name]=Domain();
            itsCounts[name]++;
		}
	}
	
	void Params::update(const string& name, const double ip) 
	{
		if(!has(name)) {
			throw(std::invalid_argument("Parameter " + name + " does not already exist"));
		}
		else {
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsDomains[name]=Domain();
            itsCounts[name]++;
		}
	}
	
	const uint Params::size() const
	{
		return static_cast<uint>(itsFree.size());
	}
	
	bool Params::has(const string& name) const 
	{
		return itsArrays.count(name)>0;
	}		

	bool Params::isScalar(const string& name) const 
	{
		return itsArrays[name].nelements()==1;
	}		

	const casa::Array<double>& Params::value(const string& name) const 
	{
		return itsArrays[name];
	}		
	
	casa::Array<double>& Params::value(const string& name) 
	{
        itsCounts[name]++;
		return itsArrays[name];
	}		
	
	const double Params::scalarValue(const string& name) const
	{
		if(!isScalar(name)) {
			throw(std::invalid_argument("Parameter " + name + " is not scalar"));
		}
		return itsArrays[name](casa::IPosition(1,0));
	}		
	
	double Params::scalarValue(const string& name) 
	{
		if(!isScalar(name)) {
			throw(std::invalid_argument("Parameter " + name + " is not scalar"));
		}
        itsCounts[name]++;
		return itsArrays[name](casa::IPosition(1,0));
	}		
	
	const Domain& Params::domain(const string& name) const 
	{
		return itsDomains[name];
	}		
	
	Domain& Params::domain(const string& name) 
	{
		return itsDomains[name];
	}		
	
	bool Params::isCongruent(const Params& other) const
	{
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(other.itsFree.count(iter->first)==0) {
				return false;
			}
		}
		return true;
	}

	void Params::merge(const Params& other)
	{
		std::vector<string> names(other.names());
		std::vector<string>::iterator iter;
		for(iter = names.begin(); iter != names.end(); iter++) {
			if(!has(*iter)) {
				itsArrays[*iter]=other.itsArrays[*iter];
				itsFree[*iter]=other.itsFree[*iter];
				itsDomains[*iter]=other.itsDomains[*iter];
                itsCounts[*iter]++;
			}
		}
	}

	vector<string> Params::names() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			names.push_back(iter->first);
		}
		return names;
	}

	vector<string> Params::freeNames() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(itsFree[iter->first]) names.push_back(iter->first);
		}
		return names;
	}

	vector<string> Params::fixedNames() const
	{
		vector<string> names;
		std::map<string,bool>::iterator iter;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(!itsFree[iter->first]) names.push_back(iter->first);
		}
		return names;
	}
	
	vector<string> Params::completions(const string& pattern) const
	{
		casa::Regex regex(casa::Regex::fromPattern(pattern+"*"));
		casa::Regex sub(casa::Regex::fromPattern(pattern));
		vector<string> completions;
		std::map<string,bool>::iterator iter;
		uint ncomplete=0;
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(casa::String(iter->first).matches(regex)) {
				casa::String complete(iter->first);
				complete.gsub(sub, "");
				completions.push_back(complete);
				ncomplete++;
			}
		}
		return completions;
	}

	void Params::reset()
	{
		itsArrays.clear();
		itsDomains.clear();
		itsFree.clear();
        itsCounts.clear();
	}
	
	ostream& operator<<(ostream& os, const Params& params) {

		vector<string> names(params.names());
		vector<string>::iterator it;
		for(it = names.begin(); it != names.end(); it++) {
			os << *it << " : ";
			if(params.isScalar(*it)) {
				os << " (scalar) " << params.scalarValue(*it);
			}
			else {
				os << " (array : shape " << params.value(*it).shape() << ") ";
			}
			if(params.isFree(*it)) {
				os << " (free)" << std::endl;
			}
			else {
				os << " (fixed)" << std::endl;
			}
		}
		return os;
	}
    
    int Params::count(const string& name) const {
        return itsCounts[name];
    }

	
}
}
