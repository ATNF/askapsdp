/// @file
///
/// ParamsTable: Base class for storing and retrieving Params by 
/// domain specification.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#ifndef SYNPARAMSTABLE_H_
#define SYNPARAMSTABLE_H_

namespace conrad
{
namespace synthesis
{

class Params;
class Domain;

class ParamsTable
{
public:
	ParamsTable();
	
	virtual ~ParamsTable();
	
	/// Get the parameters for a specified domain
	/// @param ip Template of parameters - must match
	/// @param domain Domain of validity of parameters
	virtual bool getParameters(Params& ip, const Domain& domain) const;

	/// Set the parameters for a given domain
	/// @param ip Parameters to set
	/// @param domain Domain of validity for parameters
	virtual bool setParameters (const Params& ip, const Domain& domain);

};

}
}

#endif /*PARAMSTABLE_H_*/
