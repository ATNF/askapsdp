/// @file
///
/// IEqParams: represent a set of parameters for an imaging equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQPARAMS_H_
#define IEQPARAMS_H_

#include <map>
#include <string>

using std::string;
using std::map;

#include "IEqParam.h"

namespace conrad {

class IEqParams {
public:
	/// Create from a table
	/// @param parmtable Name of parameter table
	IEqParams(string& parmtable);
		
	/// Empty constructor
	IEqParams();
	
	/// Assignment operator
	IEqParams& operator=(const IEqParams& other);
	
	/// Copy constructor
	IEqParams(const IEqParams& other);

	/// Copy constructor with select
	/// @param regex Regular expression for names
	IEqParams(const IEqParams& other, const string& regex);
	
	/// Add an ImagingParam
	/// @param ip IEqParam to be added
	void add(const string& name, const IEqParam& ip);
		
	/// Store as a table
	/// @param parmtable Name of table to be saved
	void saveAsTable(const string& parmtable) const;
	
	~IEqParams();
	
private:
	map<string, IEqParam> itsParams;
};

}

#endif /*IEQPARAMS_H_*/
