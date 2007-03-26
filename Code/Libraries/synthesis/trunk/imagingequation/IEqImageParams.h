/// @file
///
/// IEqImageParams: represent a set of parameters for an imaging equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQIMAGEPARAMS_H_
#define IEQIMAGEPARAMS_H_

#include <map>
#include <string>
#include <ostream>

using std::string;
using std::map;

#include "IEqImageParam.h"

namespace conrad {

class IEqImageParams : public map<string, IEqImageParam> {
public:

	IEqImageParams() {};
	
	/// Create from a table
	/// @param parmtable Name of parameter table
	IEqImageParams(const string& parmtable);
		
	/// Add an ImagingParam
	/// @param name Name of param to be added
	/// @param ip IEqImageParam to be added
	void add(const string& name, const IEqImageParam& ip);
		
	/// Store as a table
	/// @param parmtable Name of table to be saved
	void saveAsTable(const string& parmtable) const;
	
	/// Initialize derivatives
	void initDerivatives();
	
	/// Add derivatives
	void addDerivatives(const IEqImageParams& ip);
	
	// Write out IEqImageParams
	friend std::ostream& operator<<(std::ostream& os, const IEqImageParams& ip) {
	  for (IEqImageParams::const_iterator iter=ip.begin();iter!=ip.end();iter++) {
		  os << (*iter).first << " " << (*iter).second << std::endl;
	  }
	  return os;
	}

	
private:
};

}

#endif /*IEQPARAMS_H_*/
