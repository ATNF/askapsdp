#include <measurementequation/MEDomain.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>

#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

using std::ostream;
using std::string;
using std::vector;

namespace conrad
{
namespace synthesis
{

	/// Make an empty domain
	MEDomain::MEDomain()
	{
	}
	
	/// Assignment operator
	MEDomain& MEDomain::operator=(const MEDomain& other)
	{
		if(this!=&other) {
			itsNames=other.itsNames;
			itsStart=other.itsStart;
			itsEnd=other.itsEnd;
			itsCells=other.itsCells;
		}
	}
	
	/// Copy constructor
	MEDomain::MEDomain(const MEDomain& other)
	{
		operator=(other);
	}

	MEDomain::~MEDomain() 
	{
		itsNames.clear();
		itsStart.clear();
		itsEnd.clear();
		itsCells.clear();
	}
	
	void MEDomain::add(const string& name, const double start, const double end, const int cells)
	{
		if(has(name)) {
			throw(std::invalid_argument("Axis " + name + " already exists"));
		}
		else {
			itsNames.push_back(name);
			itsStart.push_back(start);
			itsEnd.push_back(end);
			itsCells.push_back(cells);
		}
	}
	
	/// Has this axis?
	/// @param name Name of axis
	bool MEDomain::has(const string& name) const
	{
		for (uint i=0;i<itsNames.size();i++) {
			if(itsNames[i]==name) return true;
		}
		return false;
	}
	
	int MEDomain::order(const string& name) const
	{
		for (uint i=0;i<itsNames.size();i++) {
			if(itsNames[i]==name) return i;
		}
		throw(std::invalid_argument("Axis " + name + " does not exist"));
	}
	
	const std::vector<string>& MEDomain::names() const
	{
		return itsNames;
	}
	
	const std::vector<int>& MEDomain::shape() const
	{
		return itsCells;
	}

	/// Return start value	
	/// @param name Name of axis
	double MEDomain::start(const string& name) const
	{
		return itsStart[order(name)];
	}
	
	/// Return end value	
	/// @param name Name of axis
	double MEDomain::end(const string& name) const
	{
		return itsEnd[order(name)];
	}

	/// Return number of cells
	/// @param name Name of axis
	int MEDomain::cells(const string& name) const
	{
		return itsCells[order(name)];
	}

	ostream& operator<<(ostream& os, const MEDomain& domain) {

		vector<string> names(domain.names());
		vector<string>::iterator it;
		for(it = names.begin(); it != names.end(); it++) {
			os << *it << " from " << domain.start(*it) << " to " << domain.end(*it)
				<< " in " << domain.cells(*it) << " cells" << std::endl;
		}
		return os;
	}


}

};
