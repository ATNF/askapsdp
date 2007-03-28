/// @file
///
/// IEqParamsBase: represent a set of parameters for an imaging equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQPARAMSBASE_H_
#define IEQPARAMSBASE_H_

#include <map>
#include <string>
#include <ostream>

using std::string;
using std::map;

namespace conrad {

template <class T>
class IEqParamsBase : public map<string, T> {
public:

	IEqParamsBase() {};
			
	/// Add an ImagingParam
	/// @param name Name of param to be added
	/// @param ip IEqParam to be added
	virtual void add(const string& name);
	virtual void add(const string& name, const T& ip);
	
	/// Return the parameter with this name
	/// @param name Name of param to be added
	virtual const T& operator()(const string& name) const;		
	virtual T& operator()(const string& name);
		
	/// Initialize derivatives
	virtual void initDerivatives();
	
	/// Add derivatives
	virtual void addDerivatives(const IEqParamsBase& ip);
		
private:
};

}

#endif /*IEQPARAMSBASE_H_*/
