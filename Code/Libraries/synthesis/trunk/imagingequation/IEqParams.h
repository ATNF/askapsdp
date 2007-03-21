/// @file
///
/// IEqParams: represent a set of parameters for an imaging equation.
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
	
	/// Assignment operator
	IEqParams& operator=(const IEqParams& other);
	
	/// Copy constructor
	IEqParams(const IEqParams& other);

	/// Copy constructor with select
	/// @param regex Regular expression for names
	IEqParams(const IEqParams& other, const casa::Regex& regex);
	
	/// Return names of parameters
	const casa::Vector<casa::String> names() const;
	
	/// Add an ImagingParam
	/// @param ip IEqParam to be added
	void add(const IEqParam& ip);
		
	/// Store as a table
	/// @param parmtable Name of table to be saved
	void saveAsTable(casa::String parmtable) const;
	
	~IEqParams();
	
protected:
	casa::Vector<casa::String> itsNames;
	casa::Vector<IEqParam> itsParams;
};

}

#endif /*IEQPARAMS_H_*/
