/// @file
///
/// IEqParamsTable: Base class for storing and retrieving IEqParams by 
/// domain specification.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#ifndef IEQPARAMSTABLE_H_
#define IEQPARAMSTABLE_H_

#include "IEqParams.h"
#include "IEqDomain.h"


namespace conrad
{

class IEqParamsTable
{
public:
	IEqParamsTable();
	
	virtual ~IEqParamsTable();
	
	/// Get the parameters for a specified domain
	/// @param ip Template of parameters - must match
	/// @param domain Domain of validity of parameters
	virtual IEqParams getParameters(const IEqParams& ip, const IEqDomain& domain) const;
	
	/// Set the parameters for a given domain
	/// @param ip Parameters to set
	/// @param domain Domain of validity for parameters
	virtual bool setParameters (const IEqParams& ip, const IEqDomain& domain);
};

}

#endif /*IEQPARAMSTABLE_H_*/
