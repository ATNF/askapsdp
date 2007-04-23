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
#include <casa/Arrays/Vector.h>

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
	void add(const string& name, const double ip=0.0);

	/// Add an Parameter
	/// @param name Name of param to be added
	/// @param ip Param to be added
	void add(const string& name, const casa::Vector<double>& ip);

	/// Add an Parameter
	/// @param name Name of param to be added
	/// @param ip Param to be added
	void add(const string& name, const casa::Vector<double>& ip,
		const MEDomain& domain);
	void add(const string& name, const double ip,
		const MEDomain& domain);

	/// Update an Parameter
	/// @param name Name of param to be updated
	/// @param ip Param to be updated
	void update(const string& name, const casa::Vector<double>& ip);
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
	const casa::Vector<double>& value(const string& name) const;		
	casa::Vector<double>& value(const string& name);

	/// Return the value for the scalar parameter with this name
	/// Throws invalid_argument if non-scalar
	/// @param name Name of param
	const double scalarValue(const string& name) const;		
	double scalarValue(const string& name);		

	/// Return the domain for the parameter with this name
	/// @param name Name of param
	const MEDomain& domain(const string& name) const;		
	MEDomain& domain(const string& name);
	
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

	friend std::ostream& operator<<(std::ostream& os, const MEParams& params);
	
private:
	mutable map<string, casa::Vector<double> > itsVectors;
	mutable map<string, MEDomain> itsDomains;
	mutable map<string, bool> itsFree;
};

}
}

#endif
