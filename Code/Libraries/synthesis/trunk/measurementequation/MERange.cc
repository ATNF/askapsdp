#include <measurementequation/MERange.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>

#include <vector>
#include <string>
#include <iostream>

using std::ostream;
using std::string;
using std::vector;

namespace conrad
{
namespace synthesis
{

	/// Make an empty domain
	MERange::MERange()
	{
	}
	
	/// Assignment operator
	MERange& MERange::operator=(const MERange& other)
	{
		if(this!=&other) {
			itsStart=other.itsStart;
			itsEnd=other.itsEnd;
			itsCells=other.itsCells;
		}
	}
	
	/// Copy constructor
	MERange::MERange(const MERange& other)
	{
		operator=(other);
	}

	MERange::~MERange() 
	{
		itsStart.clear();
		itsEnd.clear();
		itsCells.clear();
	}
	
	void MERange::add(const string& name, const double start, const double end, const int cells)
	{
		if(has(name)) {
			throw(casa::DuplError("Axis " + name + " already exists"));
		}
		else {
			itsStart[name]=start;
			itsEnd[name]=end;
			itsCells[name]=cells;
		}
	}
	
	/// Has this axis?
	/// @param name Name of axis
	bool MERange::has(const string& name) const
	{
		return itsStart.count(name)>0;
	}
	
	/// Return the possible axis names
	std::vector<string> MERange::names() const
	{
		vector<string> names;
		std::map<string,double>::iterator iter;
		for(iter = itsStart.begin(); iter != itsStart.end(); iter++) {
			names.push_back(iter->first);
		}
		return names;
	}

	/// Return start value	
	/// @param name Name of axis
	double MERange::start(const string& name) const
	{
		return itsStart[name];
	}
	
	/// Return end value	
	/// @param name Name of axis
	double MERange::end(const string& name) const
	{
		return itsEnd[name];
	}

	/// Return number of cells
	/// @param name Name of axis
	int MERange::cells(const string& name) const
	{
		return itsCells[name];
	}

	ostream& operator<<(ostream& os, const MERange& domain) {

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
