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
#include <ostream>

using std::string;
using std::map;

#include "IEqParam.h"

namespace conrad {

class IEqParams : public map<string, IEqParam>{
public:

	IEqParams() {};
			
	/// Add an ImagingParam
	/// @param name Name of param to be added
	/// @param ip IEqParam to be added
	void add(const string& name);
	void add(const string& name, const IEqParam& ip);
	
	const IEqParam& operator()(const string& name) const {
		IEqParams::const_iterator iter=(*this).find(name);
		return iter->second;
	};
		
	IEqParam& operator()(const string& name) {
		IEqParams::iterator iter=(*this).find(name);
		return iter->second;
	};
		
	/// Initialize derivatives
	void initDerivatives();
	
	/// Add derivatives
	void addDerivatives(const IEqParams& ip);
	
	// Write out IEqParams
	friend std::ostream& operator<<(std::ostream& os, const IEqParams& ip) {
	  for (IEqParams::const_iterator iter=ip.begin();iter!=ip.end();iter++) {
		  os << (*iter).first << " " << (*iter).second << std::endl;
	  }
	  return os;
	}

	
private:
};

}

#endif /*IEQPARAMS_H_*/
