/// @file
///
/// IEqParams: represent a set of parameters for imaging equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQPARAMS_H_
#define IEQPARAMS_H_

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Regex.h>
#include <casa/Arrays/Vector.h>
#include <tables/Tables/Table.h>

#include "IEqParam.h"

namespace conrad {

class IEqParams {
public:
	/// Create from a table
	/// @param parmtable Name of parameter table
	IEqParams(casa::String& parmtable);
	
	/// Empty constructor
	IEqParams();
	
	/// Return names of parameters
	casa::Vector<casa::String> names();
	
	/// Add an ImagingParam
	/// @param ip IEqParam to be added
	void add(IEqParam ip);
	
	/// Does this contain the named param?
	/// @param name Name of single IEqParam
	bool contains(const casa::String& name) const;
	
	/// Get the named parameter - an exception is thrown for multiple matches
	/// @param name Name of single IEqParam to be returned
	IEqParam& operator()(const casa::String& name);
	
	/// Get the named parameters
	/// @param regex Regular expression for names
	casa::Vector<IEqParam>& operator()(const casa::Regex& regex);
	
	/// Store as a table
	/// @param parmtable Name of table to be saved
	void saveAsTable(casa::String parmtable);
	
	~IEqParams();
	
private:
	casa::String itsNames;
	casa::Vector<IEqParam> itsParams;
};

}

#endif /*IEQPARAMS_H_*/
