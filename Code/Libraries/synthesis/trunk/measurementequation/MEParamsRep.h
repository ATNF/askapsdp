/// @file
///
/// MEParamsRep: represent a set of parameters for an imaging equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEPARAMSREP_H_
#define MEPARAMSREP_H_

#include <map>
#include <string>
#include <ostream>
#include <vector>

using std::string;
using std::map;
using std::vector;

namespace conrad {

/// TODO: Make this a scoped_ptr
template <class T>
class MEParamsRep {
public:

	MEParamsRep() {};
			
	/// Add an Parameter
	/// @param name Name of param to be added
	/// @param ip Param to be added
	void add(const string& name, const T& ip);
	
	/// Fix a parameter
	void fix(const string& name);

	/// Free a parameter
	void free(const string& name);

    /// Is this parameter free?
	bool isFree(const string& name);
	
	// Return the current values
	const vector<T>& values() const;
	vector<T>& values();
	
	// Return number of values
	const uint nelements() const;
	
	/// Return the parameter with this name
	/// @param name Name of param
	const T& value(const string& name) const;		
	T& value(const string& name);

	/// Return the index with this name
	/// @param name Name of param
	const uint operator[](const string& name) const;		

private:
	mutable map<string, uint> itsIndices;
	vector<T> itsValues;
	vector<bool> itsFree;
	bool ok();
};

}

#endif /*MEPARAMSBASE_H_*/
