/// @file
///
/// MEParams: represent a set of parameters for a measurement equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEPARAMS_H_
#define MEPARAMS_H_

#include <measurementequation/MEDomain.h>

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
namespace synthesis
{

class MEParams {
public:

	MEParams();
	
	MEParams& operator=(const MEParams& other);
	
	MEParams(const MEParams& other);
			
	/// Add an Parameter
	/// @param name Name of param to be added
	void add(const string& name);

	/// Add an Parameter
	/// @param name Name of param to be added
	/// @param ip Param to be added
	void add(const string& name, const casa::Array<double>& ip);

	/// Add an Parameter
	/// @param name Name of param to be added
	/// @param ip Param to be added
	void add(const string& name, const casa::Array<double>& ip,
		const MEDomain& domain);

	/// Update an Parameter
	/// @param name Name of param to be updated
	/// @param ip Param to be updated
	void update(const string& name, const casa::Array<double>& ip);
	
	/// Fix a parameter
	void fix(const string& name);

	/// Free a parameter
	void free(const string& name);

    /// Is this parameter free?
	bool isFree(const string& name);
	
	// Return number of values
	const uint size() const;
	
	/// Return the parameter with this name
	/// @param name Name of param
	const casa::Array<double>& value(const string& name) const;		
	casa::Array<double>& value(const string& name);
	
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
	bool isCongruent(const MEParams& other) const;
	
	/// Reset to empty
	void reset();
private:
	mutable map<string, casa::Array<double> > itsArrays;
	mutable map<string, MEDomain> itsDomains;
	mutable map<string, bool> itsFree;
};

}
}

#endif
