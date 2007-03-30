/// @file
///
/// MEParamsTable: Base class for storing and retrieving MEParams by 
/// domain specification.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#ifndef IEQPARAMSTABLE_H_
#define IEQPARAMSTABLE_H_

#include "MEParams.h"
#include "MEImageParams.h"
#include "MEDomain.h"


namespace conrad
{

class MEParamsTable
{
public:
	MEParamsTable();
	
	virtual ~MEParamsTable();
	
	/// Get the parameters for a specified domain
	/// @param ip Template of parameters - must match
	/// @param domain Domain of validity of parameters
	virtual bool getParameters(MEParams& ip, const MEDomain& domain) const;

	/// @param iip Template of parameters - must match
	/// @param domain Domain of validity of parameters
	virtual bool getParameters(MEImageParams& iip, const MEDomain& domain) const;
	
	/// Set the parameters for a given domain
	/// @param ip Parameters to set
	/// @param domain Domain of validity for parameters
	virtual bool setParameters (const MEParams& ip, const MEDomain& domain);

	/// @param ip Parameters to set
	/// @param domain Domain of validity for parameters
	virtual bool setParameters (const MEImageParams& iip, const MEDomain& domain);
};

}

#endif /*IEQPARAMSTABLE_H_*/
