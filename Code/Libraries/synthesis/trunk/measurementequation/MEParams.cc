#include <measurementequation/MEParams.h>
#include <measurementequation/MERange.h>
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
	
	MEParams::MEParams() {
	}
	
	MEParams::MEParams(const MEParams& other) {
		operator=(other);
	}
	
	MEParams& MEParams::operator=(const MEParams& other) {
		if(this!=&other) {
			itsArrays=other.itsArrays;
			itsRanges=other.itsRanges;
			itsFree=other.itsFree;
		}
		return *this;
	}
	
	bool MEParams::isFree(const string& name) {
		return itsFree[name];
	}
	
	void MEParams::free(const string& name) {
		itsFree[name]=true;
	}
	
	void MEParams::fix(const string& name) {
		itsFree[name]=false;
	}
	
	void MEParams::add(const string& name, const casa::Array<double>& ip) 
	{
		if(has(name)) {
			throw(casa::DuplError("Parameter " + name + " already exists"));
		}
		else {
			itsArrays[name]=ip;
			itsFree[name]=true;
			itsRanges[name]=MERange();
		}
	}
	
	void MEParams::add(const string& name, const casa::Array<double>& ip,
		const MERange& range)
	{
		if(has(name)) {
			throw(casa::DuplError("Parameter " + name + " already exists"));
		}
		else {
			itsArrays[name]=ip;
			itsFree[name]=true;
			itsRanges[name]=range;
		}
	}
	
	void MEParams::update(const string& name, const casa::Array<double>& ip) 
	{
		if(!has(name)) {
			throw(casa::DuplError("Parameter " + name + " does not already exist"));
		}
		else {
			itsArrays[name]=ip;
			itsFree[name]=true;
			itsRanges[name]=MERange();
		}
	}
	
	const uint MEParams::size() const
	{
		return static_cast<uint>(itsFree.size());
	}
	
	bool MEParams::has(const string& name) const 
	{
		return itsFree.count(name)>0;
	}		

	const casa::Array<double>& MEParams::value(const string& name) const 
	{
		return itsArrays[name];
	}		
	
	casa::Array<double>& MEParams::value(const string& name) 
	{
		return itsArrays[name];
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
		for(iter = itsFree.begin(); iter != itsFree.end(); iter++) {
			if(casa::String(iter->first).contains(regex)) completions.push_back(iter->first);
		}
		return completions;
	}

	void MEParams::reset()
	{
		itsArrays.clear();
		itsFree.clear();
	}
}
}