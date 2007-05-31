/// @file
///
/// Params: represent a set of parameters for a measurement equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHPARAMS_H_
#define SCIMATHPARAMS_H_

#include <fitting/Axes.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>

#include <map>
#include <vector>
#include <string>
#include <ostream>

using std::string;
using std::vector;
using std::map;

namespace conrad {
namespace scimath
{

class Params {
public:

	Params();
	
	Params& operator=(const Params& other);
	
	Params(const Params& other);
			
	/// Add an Parameter
	/// @param name Name of param to be added
	void add(const string& name, const double ip=0.0);

	/// Add an Parameter
	/// @param name Name of param to be added
	/// @param ip Param to be added
	void add(const string& name, const casa::Array<double>& ip);

	/// Add an Parameter
	/// @param name Name of param to be added
	/// @param ip Param to be added
	void add(const string& name, const casa::Array<double>& ip,
		const Axes& axes);
	void add(const string& name, const double ip,
		const Axes& axes);

	/// Update an Parameter
	/// @param name Name of param to be updated
	/// @param ip Param to be updated
	void update(const string& name, const casa::Array<double>& ip);
	void update(const string& name, const double ip);

    /// Is this parameter a scalar?
	bool isScalar(const string& name) const;
		
	/// Fix a parameter
	void fix(const string& name);

	/// Free a parameter
	void free(const string& name);

    /// Is this parameter free?
	bool isFree(const string& name) const;
	
	// Return number of values
	const uint size() const;
	
	/// Return the value for the parameter with this name
	/// @param name Name of param
	const casa::Array<double>& value(const string& name) const;		
	casa::Array<double>& value(const string& name);

	/// Return the value for the scalar parameter with this name
	/// Throws invalid_argument if non-scalar
	/// @param name Name of param
	const double scalarValue(const string& name) const;		
	double scalarValue(const string& name);		

	/// Return the domain for the parameter with this name
	/// @param name Name of param
	const Axes& axes(const string& name) const;		
	Axes& axes(const string& name);
	
	/// Return all the completions for this name
	/// @param match Match e.g. "flux.i.*"
	vector<string> completions(const string& match) const;
	
	/// Return the key names
	vector<string> names() const;

	/// Return the key names of free items
	vector<string> freeNames() const;
	
	/// Return the key names of fixed items
	vector<string> fixedNames() const;
	
	bool has(const string& name) const;

	/// Is this set congruent with another?
	bool isCongruent(const Params& other) const;

	/// Merge parameters from other into this set
	void merge(const Params& other);
		
	/// Reset to empty
	void reset();
    
    /// Count -  gives the number of accesses - use
    /// this as a aid in caching. Is incremented on every non-const
    /// access. A cache is no longer valid if the count has increased.
    int count(const string& name) const;

	friend std::ostream& operator<<(std::ostream& os, const Params& params);
	
private:
	mutable map<string, casa::Array<double> > itsArrays;
	mutable map<string, Axes> itsAxes;
	mutable map<string, bool> itsFree;
    mutable map<string, int> itsCounts;
    
};

}
}

#endif
