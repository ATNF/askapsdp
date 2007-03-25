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

class IEqParams : public map<string, IEqParam> {
public:

	IEqParams() {};
	
	/// Create from a table
	/// @param parmtable Name of parameter table
	IEqParams(const string& parmtable);
		
	/// Add an ImagingParam
	/// @param ip IEqParam to be added
	void add(const string& name, const IEqParam& ip);
		
	/// Store as a table
	/// @param parmtable Name of table to be saved
	void saveAsTable(const string& parmtable) const;
	
	/// Initialize derivatives
	void initDerivatives();
	
	/// Add derivatives
	void addDerivatives(IEqParams& ip);
	
private:
};

}

#endif /*IEQPARAMS_H_*/
